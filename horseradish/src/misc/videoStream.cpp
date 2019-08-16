#include "videoStream.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

//define this to reproduce the video as quickly as possible
//#define VIDEO_IGNORE_TIMING

namespace hr { namespace misc
{
	void VideoStream::processAVFrame()
	{
		AVPacket packet;

		if (mVideoQueueActive >= mVideoQueueMax)
			return;

		while (av_read_frame(mFormatContext, &packet) >= 0)
		{
			if (packet.stream_index == mVideoInfo.streamIndex)
			{
				int videoFrameEnded;
				avcodec_decode_video2(mVideoInfo.codecContext, mVideoInfo.videoFrameOriginal, &videoFrameEnded, &packet);

				if (videoFrameEnded != 0)
				{
					VideoFrame *framePrevious = nullptr;
					if (mVideoQueueActive >= 1)
						framePrevious = mVideoQueue + mVideoQueueActive - 1;

					auto *frameTarget = mVideoQueue + mVideoQueueActive;
					mVideoQueueActive++;

					auto videoFramePTS = mVideoInfo.videoFrameOriginal->best_effort_timestamp;
					//videoFramePTS = mVideoInfo.videoFrameOriginal->pkt_pts;
					//videoFramePTS = mVideoInfo.videoFrameOriginal->pkt_dts;
					if (videoFramePTS == AV_NOPTS_VALUE)
						videoFramePTS = 0;

					sws_scale(mVideoInfo.pixelConvertContext, mVideoInfo.videoFrameOriginal->data, mVideoInfo.videoFrameOriginal->linesize, 0, mVideoInfo.codecContext->height, frameTarget->avFrame->data, frameTarget->avFrame->linesize);

					frameTarget->framePTS = videoFramePTS;
					frameTarget->frameStart = videoFramePTS * av_q2d(mVideoInfo.stream->time_base);

					frameTarget->frameDuration = av_q2d(mVideoInfo.codecContext->time_base);
					frameTarget->frameDuration += mVideoInfo.videoFrameOriginal->repeat_pict * (frameTarget->frameDuration * 0.5);

					frameTarget->frameEnd = frameTarget->frameStart + frameTarget->frameDuration;

					if (framePrevious)
					{
						double newDuration = frameTarget->frameStart - framePrevious->frameStart;
						if (newDuration > framePrevious->frameDuration)
						{
							framePrevious->frameDuration = newDuration;
							framePrevious->frameEnd = framePrevious->frameStart + framePrevious->frameDuration;
						}
					}
				}

				av_free_packet(&packet);

				if (videoFrameEnded != 0)
					break;

				continue;
			}

			if (mAudioInfo.codecContext && (packet.stream_index == mAudioInfo.streamIndex))
			{
				av_free_packet(&packet);
				continue;
			}

			av_free_packet(&packet);
		}
	}

	void VideoStream::recycleVideoFrameQueue()
	{
		if (mVideoQueueMax <= 1)
			return;

		VideoFrame videoFrameTemp;
		memcpy(&videoFrameTemp, mVideoQueue + 0, sizeof(VideoFrame));

		for (size_t i = 0; i < (mVideoQueueMax - 1); i++)
			memcpy(mVideoQueue + i, mVideoQueue + i + 1, sizeof(VideoFrame));

		memcpy(mVideoQueue + mVideoQueueMax - 1, &videoFrameTemp, sizeof(VideoFrame));
	}

	VideoStream::VideoFrame* VideoStream::getLatestFrame() const
	{
		if (mVideoQueueActive == 0)
			return nullptr;

		if (mVideoQueueActive == 1)
			return (mVideoQueue + 0);

		auto frameFinal = (mVideoQueue + 0);
		for (size_t i = 1; i < mVideoQueueActive; i++)
		{
			if (mVideoQueue[i].framePTS > frameFinal->framePTS)
				frameFinal = mVideoQueue + i;
		}

		return frameFinal;
	}

	VideoStream::VideoStream(size_t maxFramesQueue, AVPixelFormat frameTargetPixelFormat, const char * const videoFilePath)
	{
		mVideoQueueMax = (maxFramesQueue < 2) ? 2 : maxFramesQueue;
		mVideoQueue = new VideoFrame[mVideoQueueMax];

		if (avformat_open_input(&mFormatContext, videoFilePath, nullptr, nullptr) != 0)
			return;

		if (avformat_find_stream_info(mFormatContext, nullptr) < 0)
			return;

		for (size_t i = 0; i < mFormatContext->nb_streams; i++)
		{
			if ((mVideoInfo.codecContext == nullptr) && (mFormatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO))
			{
				mVideoInfo.stream = mFormatContext->streams[i];
				mVideoInfo.streamIndex = i;
				mVideoInfo.codecContext = mVideoInfo.stream->codec;
				continue;
			}

			if ((mAudioInfo.codecContext == nullptr) && (mFormatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO))
			{
				mAudioInfo.streamIndex = i;
				mAudioInfo.codecContext = mFormatContext->streams[i]->codec;
				continue;
			}
		}

		if (!mVideoInfo.codecContext)
			return;

		mVideoInfo.codec = avcodec_find_decoder(mVideoInfo.codecContext->codec_id);
		if (!mVideoInfo.codec)
			return;

		if (avcodec_open2(mVideoInfo.codecContext, mVideoInfo.codec, nullptr) < 0)
			return;

		mVideoInfo.avgFrameRate = av_q2d(mVideoInfo.stream->avg_frame_rate);

		mVideoInfo.pixelConvertContext = sws_getContext(mVideoInfo.codecContext->width, mVideoInfo.codecContext->height, mVideoInfo.codecContext->pix_fmt, mVideoInfo.codecContext->width, mVideoInfo.codecContext->height, frameTargetPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

		if (mAudioInfo.codecContext)
		{
			mAudioInfo.codec = avcodec_find_decoder(mAudioInfo.codecContext->codec_id);
			if (!mAudioInfo.codec)
				return;

			if (avcodec_open2(mAudioInfo.codecContext, mAudioInfo.codec, nullptr) < 0)
				return;
		}

		mVideoInfo.videoFrameOriginal = av_frame_alloc();
		if (!mVideoInfo.videoFrameOriginal)
			return;

		int frameSize = avpicture_get_size(frameTargetPixelFormat, mVideoInfo.codecContext->width, mVideoInfo.codecContext->height);
		if (frameSize < 0)
			return;

		mVideoInfo.frameBufferSize = static_cast<size_t>(frameSize);

		size_t skipQueue = 0;
		for (size_t i = 0; i < mVideoQueueMax; i++)
		{
			auto targetFrame = mVideoQueue + i - skipQueue;

			targetFrame->avFrame = av_frame_alloc();
			if (!targetFrame->avFrame)
			{
				skipQueue++;
				continue;
			}

			targetFrame->frameData = (uint8_t *)av_malloc(mVideoInfo.frameBufferSize * sizeof(uint8_t));
			if (!targetFrame->frameData)
			{
				av_frame_free(&targetFrame->avFrame);

				skipQueue++;
				continue;
			}

			avpicture_fill((AVPicture *)targetFrame->avFrame, targetFrame->frameData, frameTargetPixelFormat, mVideoInfo.codecContext->width, mVideoInfo.codecContext->height);
		}

		mVideoQueueMax -= skipQueue;
	}

	VideoStream::~VideoStream()
	{
		for (size_t i = 0; i < mVideoQueueMax; i++)
		{
			av_free(mVideoQueue[i].frameData);
			av_frame_free(&mVideoQueue[i].avFrame);
		}

		delete[] mVideoQueue;

		if (mVideoInfo.pixelConvertContext)
			sws_freeContext(mVideoInfo.pixelConvertContext);

		if (mVideoInfo.videoFrameOriginal)
			av_frame_free(&mVideoInfo.videoFrameOriginal);

		if (mAudioInfo.codecContext)
			avcodec_close(mAudioInfo.codecContext);
		if (mVideoInfo.codecContext)
			avcodec_close(mVideoInfo.codecContext);

		if (mFormatContext)
			avformat_close_input(&mFormatContext);
	}

	void VideoStream::Initialize()
	{
		static bool avInitialized = false;
		if (avInitialized)
			return;

		av_register_all();

		avInitialized = true;
	}

	void VideoStream::process()
	{
		processAVFrame();
	}

	bool VideoStream::goToBeginning()
	{
		int streamIndex = -1;
		if (mVideoInfo.streamIndex >= 0)
			streamIndex = mVideoInfo.streamIndex;
		else if (mAudioInfo.streamIndex >= 0)
			streamIndex = mAudioInfo.streamIndex;

		int64_t seekTarget = 0;
		if (streamIndex >= 0)
		{
			//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, mFormatContext->streams[streamIndex]->time_base);

			AVRational aux = { 1, AV_TIME_BASE };
			seekTarget = av_rescale_q(0, aux, mFormatContext->streams[streamIndex]->time_base);
		}

		if (av_seek_frame(mFormatContext, streamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
			return false;

		avcodec_flush_buffers(mVideoInfo.codecContext);
		if (mAudioInfo.codecContext)
			avcodec_flush_buffers(mAudioInfo.codecContext);

		mVideoQueueActive = 0;

		processAVFrame();

		mTimerInfo.timer.reStart();
		mTimerInfo.timestampS = 0.0;

		return true;
	}

	bool VideoStream::goToTime(double seconds)
	{
		int streamIndex = -1;
		if (mVideoInfo.streamIndex >= 0)
			streamIndex = mVideoInfo.streamIndex;
		else if (mAudioInfo.streamIndex >= 0)
			streamIndex = mAudioInfo.streamIndex;

		int64_t seekTarget = static_cast<int64_t>((seconds < 0.0) ? 0.0 : seconds);
		seekTarget *= AV_TIME_BASE;
		if (streamIndex >= 0)
		{
			//seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, mFormatContext->streams[streamIndex]->time_base);

			AVRational aux = { 1, AV_TIME_BASE };
			seekTarget = av_rescale_q(seekTarget, aux, mFormatContext->streams[streamIndex]->time_base);
		}

		auto lastFrame = getLatestFrame();

		if (av_seek_frame(mFormatContext, streamIndex, seekTarget, ((lastFrame != nullptr) && (lastFrame->framePTS > seekTarget)) ? AVSEEK_FLAG_BACKWARD : 0) < 0)
			return false;

		avcodec_flush_buffers(mVideoInfo.codecContext);
		if (mAudioInfo.codecContext)
			avcodec_flush_buffers(mAudioInfo.codecContext);

		mVideoQueueActive = 0;

		processAVFrame();

		if (mVideoQueueActive > 0)
			mTimerInfo.timer.setS(mVideoQueue[0].frameStart);
		else
			mTimerInfo.timer.setS(seconds);
		mTimerInfo.timestampS = mTimerInfo.timer.getTimeS();

		return true;
	}

	const void* VideoStream::getFrame(bool &clockIsBehind, hr::hInt64 &frameID, double &frameDurationS)
	{
		clockIsBehind = false;
		frameID = -1;
		frameDurationS = 0.0;

		mTimerInfo.timestampS = mTimerInfo.timer.getTimeS();

		while (true)
		{
			if (mVideoQueueActive <= 0)
			{
				processAVFrame();

				if (mVideoQueueActive <= 0)
					return nullptr;
			}

	#if !defined(VIDEO_IGNORE_TIMING)
			if (mTimerInfo.timestampS < mVideoQueue[0].frameStart)
			{
				clockIsBehind = true;
				return nullptr;
			}

			if (mTimerInfo.timestampS < mVideoQueue[0].frameEnd)
			{
				auto targetFrame = mVideoQueue + 0;

				frameID = targetFrame->framePTS;
				frameDurationS = targetFrame->frameEnd - mTimerInfo.timer.getTimeS();

				return targetFrame->frameData;
			}

			recycleVideoFrameQueue();
			mVideoQueueActive--;
	#else
			frameID = mTimerInfo.timer.getTimeIntMS();
			frameDurationS = 0.0;

			auto frameData = mVideoQueue[0].frameData;

			recycleVideoFrameQueue();
			mVideoQueueActive--;

			return frameData;
	#endif
		}
	}

	double VideoStream::getFrameDuration(const hr::hInt64 &frameID) const
	{
		if (mVideoQueueActive <= 0)
			return 0.0;

		auto targetFrame = mVideoQueue + 0;
		if (targetFrame->framePTS != frameID)
			return 0.0;

		return (targetFrame->frameEnd - mTimerInfo.timer.getTimeS());
	}

	bool VideoStream::isValid() const
	{
		return (mVideoInfo.videoFrameOriginal && (mVideoQueueMax > 0));
	}

	bool VideoStream::hasAudio() const
	{
		return (mAudioInfo.codec != nullptr);
	}

	double VideoStream::getTimeStampDelta() const
	{
		return (mTimerInfo.timer.getTimeS() - mTimerInfo.timestampS);
	}

	size_t VideoStream::getVideoFrameDataSize() const
	{
		return mVideoInfo.frameBufferSize;
	}

	double VideoStream::getVideoFrameAspectRatio() const
	{
		if (mVideoInfo.stream && (mVideoInfo.stream->sample_aspect_ratio.num != 0))
			return av_q2d(mVideoInfo.stream->sample_aspect_ratio);

		if (mVideoInfo.codecContext && (mVideoInfo.codecContext->sample_aspect_ratio.num != 0))
			return av_q2d(mVideoInfo.codecContext->sample_aspect_ratio);

		return -1.0;
	}

	void VideoStream::getVideoDims(size_t &videoWidth, size_t &videoHeight) const
	{
		videoWidth = videoHeight = 0;

		if (mVideoInfo.codecContext)
		{
			videoWidth = static_cast<size_t>(mVideoInfo.codecContext->width);
			videoHeight = static_cast<size_t>(mVideoInfo.codecContext->height);
		}
	}

	hr::Rectangle<int> VideoStream::getVideoRect(size_t winWidth, size_t winHeight, bool maintainAspectRatio) const
	{
		if (!mVideoInfo.codecContext)
			return hr::Rectangle<int>();

		size_t videoWidth, videoHeight;
		getVideoDims(videoWidth, videoHeight);

		if (!maintainAspectRatio)
			return hr::Rectangle<int>(0, 0, winWidth, winHeight);

		auto videoScale = static_cast<float>(videoWidth) / static_cast<float>(videoHeight);
		auto videoAspectRatio = getVideoFrameAspectRatio();
		if (videoAspectRatio > 0.0)
			videoScale *= videoAspectRatio;

		if ((winWidth / videoScale) > winHeight)
		{
			auto videoNewWidth = static_cast<float>(winHeight) * videoScale;

			return hr::Rectangle<int>(
				hr::Math::ftoi((static_cast<float>(winWidth) - videoNewWidth) * 0.5f),
				0,
				hr::Math::ftoi(videoNewWidth),
				winHeight
				);
		}
		else
		{
			auto videoNewHeight = static_cast<float>(winWidth) / videoScale;

			return hr::Rectangle<int>(
				0,
				hr::Math::ftoi((static_cast<float>(winHeight) - videoNewHeight) * 0.5f),
				winWidth,
				hr::Math::ftoi(videoNewHeight)
				);
		}
	}

	hr::Rectangle<int> VideoStream::getVideoRectCenter(float scale, size_t winWidth, size_t winHeight) const
	{
		if (!mVideoInfo.codecContext)
			return hr::Rectangle<int>();

		size_t videoWidth, videoHeight;
		getVideoDims(videoWidth, videoHeight);

		auto videoScale = static_cast<float>(videoWidth) / static_cast<float>(videoHeight);
		auto videoAspectRatio = getVideoFrameAspectRatio();
		if (videoAspectRatio > 0.0)
			videoScale *= videoAspectRatio;

		videoWidth = hr::Math::ftoi(static_cast<float>(videoWidth) * scale);
		videoHeight = hr::Math::ftoi(static_cast<float>(videoWidth) / videoScale);

		return hr::Rectangle<int>(
			hr::Math::ftoi(static_cast<float>(winWidth - videoWidth) * 0.5f),
			hr::Math::ftoi(static_cast<float>(winHeight - videoHeight) * 0.5f),
			videoWidth,
			videoHeight
		);
	}

}}