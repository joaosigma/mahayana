#include "videoStream.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

//DEBUG: definir esta constante para reproduzir o video o mais rapidamente possível
//#define VIDEO_IGNORE_TIMING

bool VideoStream::avInitialized = false;

void VideoStream::processAVFrame()
{
	AVPacket packet;

	if (this->videoQueueActive >= this->videoQueueMax)
		return;

	while (av_read_frame(this->formatContext, &packet) >= 0)
	{
		if (packet.stream_index == this->videoInfo.streamIndex)
		{
			int videoFrameTerminou;

			avcodec_decode_video2(this->videoInfo.codecContext, this->videoInfo.videoFrameOriginal, &videoFrameTerminou, &packet);

			if (videoFrameTerminou != 0)
			{
				VideoFrame *frameAnterior, *frameAlvo;
				uint64_t videoFramePTS;

				frameAnterior = nullptr;
				if (this->videoQueueActive >= 1)
					frameAnterior = this->videoQueue + this->videoQueueActive - 1;

				frameAlvo = this->videoQueue + this->videoQueueActive;
				this->videoQueueActive++;

				videoFramePTS = this->videoInfo.videoFrameOriginal->best_effort_timestamp;
				//videoFramePTS = this->videoInfo.videoFrameOriginal->pkt_pts;
				//videoFramePTS = this->videoInfo.videoFrameOriginal->pkt_dts;
				if (videoFramePTS == AV_NOPTS_VALUE)
					videoFramePTS = 0;

				sws_scale(this->videoInfo.pixelConvertContext, this->videoInfo.videoFrameOriginal->data, this->videoInfo.videoFrameOriginal->linesize, 0, this->videoInfo.codecContext->height, frameAlvo->avFrame->data, frameAlvo->avFrame->linesize);

				frameAlvo->framePTS = videoFramePTS;
				frameAlvo->frameStart = videoFramePTS * av_q2d(this->videoInfo.stream->time_base);

				frameAlvo->frameDuration = av_q2d(this->videoInfo.codecContext->time_base);
				frameAlvo->frameDuration += this->videoInfo.videoFrameOriginal->repeat_pict * (frameAlvo->frameDuration * 0.5);

				frameAlvo->frameEnd = frameAlvo->frameStart + frameAlvo->frameDuration;

				if (frameAnterior != nullptr)
				{
					double newDuration = frameAlvo->frameStart - frameAnterior->frameStart;
					if (newDuration > frameAnterior->frameDuration)
					{
						frameAnterior->frameDuration = newDuration;
						frameAnterior->frameEnd = frameAnterior->frameStart + frameAnterior->frameDuration;
					}
				}
			}

			av_free_packet(&packet);

			if (videoFrameTerminou != 0)
				break;

			continue;
		}

		if ((this->audioInfo.codecContext != nullptr) && (packet.stream_index == this->audioInfo.streamIndex))
		{
			av_free_packet(&packet);
			continue;
		}

		av_free_packet(&packet);
	}
}

void VideoStream::recycleVideoFrameQueue()
{
	VideoFrame videoFrameTemp;

	if (this->videoQueueMax <= 0)
		return;

	memcpy(&videoFrameTemp, this->videoQueue + 0, sizeof(VideoFrame));

	for (int i = 0; i < (this->videoQueueMax - 1); i++)
		memcpy(this->videoQueue + i, this->videoQueue + i + 1, sizeof(VideoFrame));

	memcpy(this->videoQueue + this->videoQueueMax - 1, &videoFrameTemp, sizeof(VideoFrame));
}

VideoStream::VideoFrame* VideoStream::getLatestFrame() const
{
	if (this->videoQueueActive <= 0)
		return nullptr;

	if (this->videoQueueActive == 1)
		return (this->videoQueue + 0);

	auto frameFinal = (this->videoQueue + 0);
	for (int i = 1; i < this->videoQueueActive; i++)
	{
		if (this->videoQueue[i].framePTS > frameFinal->framePTS)
			frameFinal = this->videoQueue + i;
	}

	return frameFinal;
}

VideoStream::VideoStream(const unsigned int maxFramesQueue, const PixelFormat frameTargetPixelFormat, const char * const videoFilePath)
{
	this->formatContext = nullptr;
	this->timerInfo.timestampS = 0.0;
	memset(&this->videoInfo, 0, sizeof(VideoInfo));
	memset(&this->audioInfo, 0, sizeof(AudioInfo));
	this->videoInfo.streamIndex = -1;
	this->audioInfo.streamIndex = -1;

	this->videoQueueMax = (maxFramesQueue < 2) ? 2 : maxFramesQueue;
	this->videoQueueActive = 0;
	this->videoQueue = new VideoFrame[this->videoQueueMax];

	for (int i = 0; i < this->videoQueueMax; i++)
		memset(this->videoQueue + i, 0, sizeof(VideoFrame));

	if (avformat_open_input(&this->formatContext, videoFilePath, nullptr, nullptr) != 0)
		return;

	if (av_find_stream_info(this->formatContext) < 0)
		return;

	for (int i = 0; i < this->formatContext->nb_streams; i++)
	{
		if ((this->videoInfo.codecContext == nullptr) && (this->formatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO))
		{
			this->videoInfo.stream = this->formatContext->streams[i];
			this->videoInfo.streamIndex = i;
			this->videoInfo.codecContext = this->videoInfo.stream->codec;
			continue;
		}

		if ((this->audioInfo.codecContext == nullptr) && (this->formatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO))
		{
			this->audioInfo.streamIndex = i;
			this->audioInfo.codecContext = this->formatContext->streams[i]->codec;
			continue;
		}
	}

	if (this->videoInfo.codecContext == nullptr)
		return;

	this->videoInfo.codec = avcodec_find_decoder(this->videoInfo.codecContext->codec_id);
	if (this->videoInfo.codec == nullptr)
		return;

	if (avcodec_open2(this->videoInfo.codecContext, this->videoInfo.codec, nullptr) < 0)
		return;

	this->videoInfo.avgFrameRate = av_q2d(this->videoInfo.stream->avg_frame_rate);

	this->videoInfo.pixelConvertContext = sws_getContext(this->videoInfo.codecContext->width, this->videoInfo.codecContext->height, this->videoInfo.codecContext->pix_fmt, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height, frameTargetPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

	if (this->audioInfo.codecContext != nullptr)
	{
		this->audioInfo.codec = avcodec_find_decoder(this->audioInfo.codecContext->codec_id);
		if (this->audioInfo.codec == nullptr)
			return;

		if (avcodec_open2(this->audioInfo.codecContext, this->audioInfo.codec, nullptr) < 0)
			return;
	}

	this->videoInfo.videoFrameOriginal = avcodec_alloc_frame();
	if (this->videoInfo.videoFrameOriginal == nullptr)
		return;

	this->videoInfo.frameBufferSize = avpicture_get_size(frameTargetPixelFormat, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height);

	for (int i = 0; i < this->videoQueueMax; i++)
	{
		this->videoQueue[i].avFrame = avcodec_alloc_frame();
		if (this->videoQueue[i].avFrame == nullptr)
		{
			this->videoQueueMax--;
			i--;
			continue;
		}

		this->videoQueue[i].frameData = (uint8_t *)av_malloc(this->videoInfo.frameBufferSize * sizeof(uint8_t));
		if (this->videoQueue[i].frameData == nullptr)
		{
			av_free(this->videoQueue[i].avFrame);
			this->videoQueue[i].avFrame = nullptr;

			this->videoQueueMax--;
			i--;
			continue;
		}

		avpicture_fill((AVPicture *)this->videoQueue[i].avFrame, this->videoQueue[i].frameData, frameTargetPixelFormat, this->videoInfo.codecContext->width, this->videoInfo.codecContext->height);
	}
}

VideoStream::~VideoStream()
{
	for (int i = 0; i < this->videoQueueMax; i++)
	{
		av_free(this->videoQueue[i].frameData);
		av_free(this->videoQueue[i].avFrame);
	}

	delete[] this->videoQueue;

	if (this->videoInfo.pixelConvertContext != nullptr)
		sws_freeContext(this->videoInfo.pixelConvertContext);

	if (this->videoInfo.videoFrameOriginal != nullptr)
		av_free(this->videoInfo.videoFrameOriginal);

	if (this->audioInfo.codecContext != nullptr)
		avcodec_close(this->audioInfo.codecContext);
	if (this->videoInfo.codecContext != nullptr)
		avcodec_close(this->videoInfo.codecContext);

	if (this->formatContext != nullptr)
		av_close_input_file(this->formatContext);

	this->formatContext = nullptr;
	this->timerInfo.timestampS = 0.0;
	memset(&this->videoInfo, 0, sizeof(VideoInfo));
	memset(&this->audioInfo, 0, sizeof(AudioInfo));
	this->videoInfo.streamIndex = -1;
	this->audioInfo.streamIndex = -1;
	this->videoQueue = nullptr;
	this->videoQueueActive = 0;
	this->videoQueueMax = 0;
}

void VideoStream::Initialize()
{
	if (VideoStream::avInitialized == true)
		return;

	av_register_all();

	VideoStream::avInitialized = true;
}

void VideoStream::Process()
{
	processAVFrame();
}

bool VideoStream::GoToBeginning()
{
	int streamIndex = -1;
	if (this->videoInfo.streamIndex >= 0)
		streamIndex = this->videoInfo.streamIndex;
	else if (this->audioInfo.streamIndex >= 0)
		streamIndex = this->audioInfo.streamIndex;

	int64_t seekTarget = 0;
	if (streamIndex >= 0)
	{
		//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, this->formatContext->streams[streamIndex]->time_base);

		AVRational aux = { 1, AV_TIME_BASE };
		seekTarget = av_rescale_q(0, aux, this->formatContext->streams[streamIndex]->time_base);
	}

	if (av_seek_frame(this->formatContext, streamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
		return false;

	avcodec_flush_buffers(this->videoInfo.codecContext);
	if (this->audioInfo.codecContext != nullptr)
		avcodec_flush_buffers(this->audioInfo.codecContext);

	this->videoQueueActive = 0;

	processAVFrame();

	this->timerInfo.timer.reStart();
	this->timerInfo.timestampS = 0.0;

	return true;
}

bool VideoStream::GoToTime(const double seconds)
{
	int streamIndex = -1;
	if (this->videoInfo.streamIndex >= 0)
		streamIndex = this->videoInfo.streamIndex;
	else if (this->audioInfo.streamIndex >= 0)
		streamIndex = this->audioInfo.streamIndex;

	int64_t seekTarget = (seconds < 0) ? 0 : seconds;
	seekTarget *= AV_TIME_BASE;
	if (streamIndex >= 0)
	{
		//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, this->formatContext->streams[streamIndex]->time_base);

		AVRational aux = { 1, AV_TIME_BASE };
		seekTarget = av_rescale_q(seekTarget, aux, this->formatContext->streams[streamIndex]->time_base);
	}

	auto lastFrame = getLatestFrame();

	if (av_seek_frame(this->formatContext, streamIndex, seekTarget, ((lastFrame != nullptr) && (lastFrame->framePTS > seekTarget)) ? AVSEEK_FLAG_BACKWARD : 0) < 0)
		return false;

	avcodec_flush_buffers(this->videoInfo.codecContext);
	if (this->audioInfo.codecContext != nullptr)
		avcodec_flush_buffers(this->audioInfo.codecContext);

	this->videoQueueActive = 0;

	processAVFrame();

	if (this->videoQueueActive > 0)
		this->timerInfo.timer.setS(this->videoQueue[0].frameStart);
	else
		this->timerInfo.timer.setS(seconds);
	this->timerInfo.timestampS = this->timerInfo.timer.getTimeS();

	return true;
}

const void* VideoStream::GetFrame(bool &clockIsBehind, HorseRadish::hInt64 &frameID, double &frameDurationS)
{
	clockIsBehind = false;
	frameID = -1;
	frameDurationS = 0.0;

	this->timerInfo.timestampS = this->timerInfo.timer.getTimeS();

	while (true)
	{
		if (this->videoQueueActive <= 0)
		{
			processAVFrame();

			if (this->videoQueueActive <= 0)
				return nullptr;
		}

#ifndef VIDEO_IGNORE_TIMING
		if (this->timerInfo.timestampS < this->videoQueue[0].frameStart)
		{
			clockIsBehind = true;
			return nullptr;
		}

		if (this->timerInfo.timestampS < this->videoQueue[0].frameEnd)
		{
			auto frameAlvo = this->videoQueue + 0;

			frameID = frameAlvo->framePTS;
			frameDurationS = frameAlvo->frameEnd - this->timerInfo.timer.getTimeS();

			return frameAlvo->frameData;
		}

		recycleVideoFrameQueue();
		this->videoQueueActive--;
#else
		auto timerValue = this->timerInfo.timer.GetTimeIntMS(timerValue);
		frameID = timerValue;
		frameDurationS = 0.0;

		auto frameData = this->videoQueue[0].frameData;

		recycleVideoFrameQueue();
		this->videoQueueActive--;

		return frameData;
#endif
	}
}

double VideoStream::GetFrameDuration(const HorseRadish::hInt64 &frameID)
{
	if (this->videoQueueActive <= 0)
		return 0.0;

	auto frameAlvo = this->videoQueue + 0;

	if (frameAlvo->framePTS != frameID)
		return 0.0;

	return (frameAlvo->frameEnd - this->timerInfo.timer.getTimeS());
}

bool VideoStream::IsValid() const
{
	return ((this->videoInfo.videoFrameOriginal != nullptr) && (this->videoQueueMax > 0));
}

bool VideoStream::HasAudio() const
{
	return (this->audioInfo.codec != nullptr);
}

double VideoStream::GetTimeStampDelta()
{
	return (this->timerInfo.timer.getTimeS() - this->timerInfo.timestampS);
}

int VideoStream::GetVideoFrameDataSize() const
{
	return this->videoInfo.frameBufferSize;
}

double VideoStream::GetVideoFrameAspectRatio() const
{
	if ((this->videoInfo.stream != nullptr) && (this->videoInfo.stream->sample_aspect_ratio.num != 0))
		return av_q2d(this->videoInfo.stream->sample_aspect_ratio);
	if ((this->videoInfo.codecContext != nullptr) && (this->videoInfo.codecContext->sample_aspect_ratio.num != 0))
		return av_q2d(this->videoInfo.codecContext->sample_aspect_ratio);

	return -1.0;
}

void VideoStream::GetVideoDims(int &videoWidth, int &videoHeight) const
{
	videoWidth = videoHeight = 0;

	if (this->videoInfo.codecContext != nullptr)
	{
		videoWidth = this->videoInfo.codecContext->width;
		videoHeight = this->videoInfo.codecContext->height;
	}
}

void VideoStream::GetVideoRect(const int winWidth, const int winHeight, const bool maintainAspectRatio, HorseRadish::Primitives2D::Rectangle<int> &rect) const
{
	int videoWidth, videoHeight;

	rect.Reset();

	if (this->videoInfo.codecContext == nullptr)
		return;

	this->GetVideoDims(videoWidth, videoHeight);

	if (maintainAspectRatio == false)
	{
		rect.x = 0;
		rect.y = 0;
		rect.width = winWidth;
		rect.height = winHeight;
		return;
	}

	float videoScale = ((float)videoWidth) / ((float)videoHeight);
	float videoAspectRatio = this->GetVideoFrameAspectRatio();
	if (videoAspectRatio > 0)
		videoScale *= videoAspectRatio;

	if ((winWidth / videoScale) > winHeight)
	{
		float videoNewWidth = ((float)winHeight) * videoScale;

		rect.x = HorseRadish::Math::ftoi((((float)winWidth) - videoNewWidth) * 0.5f);
		rect.y = 0;
		rect.width = HorseRadish::Math::ftoi(videoNewWidth);
		rect.height = winHeight;
	}
	else
	{
		float videoNewHeight = ((float)winWidth) / videoScale;

		rect.x = 0;
		rect.y = HorseRadish::Math::ftoi((((float)winHeight) - videoNewHeight) * 0.5f);
		rect.width = winWidth;
		rect.height = HorseRadish::Math::ftoi(videoNewHeight);
	}
}

void VideoStream::GetVideoRectCenter(const float scale, const int winWidth, const int winHeight, HorseRadish::Primitives2D::Rectangle<int> &rect) const
{
	int videoWidth, videoHeight;

	rect.Reset();

	if (this->videoInfo.codecContext == nullptr)
		return;

	this->GetVideoDims(videoWidth, videoHeight);

	float videoScale = ((float)videoWidth) / ((float)videoHeight);
	float videoAspectRatio = this->GetVideoFrameAspectRatio();
	if (videoAspectRatio > 0)
		videoScale *= videoAspectRatio;

	videoWidth = HorseRadish::Math::ftoi(((float)videoWidth) * scale);
	videoHeight = HorseRadish::Math::ftoi(((float)videoWidth) / videoScale);

	rect.x = HorseRadish::Math::ftoi(((float)(winWidth - videoWidth)) * 0.5f);
	rect.y = HorseRadish::Math::ftoi(((float)(winHeight - videoHeight)) * 0.5f);;
	rect.width = videoWidth;
	rect.height = videoHeight;
}