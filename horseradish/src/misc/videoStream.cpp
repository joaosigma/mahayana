module;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

module VideoStream;

// define this to reproduce the video as quickly as possible
// #define VIDEO_IGNORE_TIMING

namespace hr::misc
{
    void VideoStream::readAVFrame()
    {
        if (mVideoQueueActive >= mVideoQueueMax)
            return;

        std::function<bool(AVPacket * packet)> processPacket;
        processPacket = [this, &processPacket](AVPacket* packet)
        {
            // try to read something
            switch (avcodec_receive_frame(mVideoInfo.codecContext, mVideoInfo.videoFrameOriginal))
            {
                // sucess (there was still data in the codec)
                case 0:
                    processAVFrame();
                    return true;

                // needs more data to be sent
                case AVERROR_EOF:
                case AVERROR(EAGAIN):
                    break;

                // can't recover
                default:
                    return false;
            }

            if (!packet)
                return false;

            // send packet to codec
            switch (avcodec_send_packet(mVideoInfo.codecContext, packet))
            {
                // success
                case 0:
                    return processPacket(nullptr);

                // can't process packet right away, need to receive frame and try again
                case AVERROR(EAGAIN):
                    return processPacket(packet);

                default:
                    return false;
            }
        };

        while (av_read_frame(mFormatContext, mVideoInfo.packet) == 0)
        {
            if (mVideoInfo.packet->stream_index == mVideoInfo.streamIndex)
            {
                auto frameRead = processPacket(mVideoInfo.packet);
                av_packet_unref(mVideoInfo.packet);

                if (!frameRead)
                    continue;
                break;
            }

            av_packet_unref(mVideoInfo.packet);
        }
    }

    void VideoStream::processAVFrame()
    {
        VideoFrame* framePrevious = nullptr;
        if (mVideoQueueActive >= 1)
            framePrevious = mVideoQueue.get() + mVideoQueueActive - 1;

        auto& frameTarget = mVideoQueue[mVideoQueueActive];
        mVideoQueueActive++;

        auto videoFramePTS = mVideoInfo.videoFrameOriginal->best_effort_timestamp;
        // videoFramePTS = mVideoInfo.videoFrameOriginal->pkt_pts;
        // videoFramePTS = mVideoInfo.videoFrameOriginal->pkt_dts;
        if (videoFramePTS == AV_NOPTS_VALUE)
            videoFramePTS = 0;

        sws_scale(mVideoInfo.pixelConvertContext, mVideoInfo.videoFrameOriginal->data, mVideoInfo.videoFrameOriginal->linesize, 0, mVideoInfo.codecContext->height,
                  frameTarget.avFrame->data, frameTarget.avFrame->linesize);

        frameTarget.framePTS = videoFramePTS;
        frameTarget.frameStart = videoFramePTS * av_q2d(mVideoInfo.stream->time_base);

        frameTarget.frameDuration = av_q2d(mVideoInfo.codecContext->time_base);
        frameTarget.frameDuration += mVideoInfo.videoFrameOriginal->repeat_pict * (frameTarget.frameDuration * 0.5);

        frameTarget.frameEnd = frameTarget.frameStart + frameTarget.frameDuration;

        if (framePrevious)
        {
            double newDuration = frameTarget.frameStart - framePrevious->frameStart;
            if (newDuration > framePrevious->frameDuration)
            {
                framePrevious->frameDuration = newDuration;
                framePrevious->frameEnd = framePrevious->frameStart + framePrevious->frameDuration;
            }
        }
    }

    void VideoStream::recycleVideoFrameQueue()
    {
        if (mVideoQueueMax <= 1)
            return;

        VideoFrame videoFrameTemp;
        std::memcpy(&videoFrameTemp, &mVideoQueue[0], sizeof(VideoFrame));

        for (size_t i = 0; i < (mVideoQueueMax - 1); i++)
            std::memcpy(&mVideoQueue[i], &mVideoQueue[i + 1], sizeof(VideoFrame));

        std::memcpy(&mVideoQueue[mVideoQueueMax - 1], &videoFrameTemp, sizeof(VideoFrame));
    }

    VideoStream::VideoFrame* VideoStream::getLatestFrame() const
    {
        if (mVideoQueueActive == 0)
            return nullptr;

        if (mVideoQueueActive == 1)
            return &mVideoQueue[0];

        auto frameFinal = &mVideoQueue[0];
        for (size_t i = 1; i < mVideoQueueActive; i++)
        {
            if (mVideoQueue[i].framePTS > frameFinal->framePTS)
                frameFinal = &mVideoQueue[i];
        }

        return frameFinal;
    }

    VideoStream::VideoStream(size_t maxFramesQueue, AVPixelFormat frameTargetPixelFormat, const char* const videoFilePath)
    {
        mVideoQueueMax = (maxFramesQueue < 2) ? 2 : maxFramesQueue;
        mVideoQueue = std::make_unique<VideoFrame[]>(mVideoQueueMax);

        // the main format ctx
        mFormatContext = avformat_alloc_context();
        if (!mFormatContext)
            return;

        // open file in the ctx
        if (avformat_open_input(&mFormatContext, videoFilePath, nullptr, nullptr) != 0)
            return;

        // check if there are any streams available
        if (avformat_find_stream_info(mFormatContext, nullptr) < 0)
            return;

        // find the video and audio streams
        for (int i = 0; i < mFormatContext->nb_streams; i++)
        {
            AVCodecParameters* codecParams = mFormatContext->streams[i]->codecpar;

            const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
            if (codec == nullptr)
                continue;

            if ((mVideoInfo.streamIndex == -1) && (codecParams->codec_type == AVMEDIA_TYPE_VIDEO))
            {
                mVideoInfo.stream = mFormatContext->streams[i];
                mVideoInfo.streamIndex = i;

                mVideoInfo.codecContext = avcodec_alloc_context3(codec);
                if (!mVideoInfo.codecContext)
                    continue;

                if (avcodec_parameters_to_context(mVideoInfo.codecContext, codecParams) < 0)
                    return;

                if (avcodec_open2(mVideoInfo.codecContext, codec, nullptr) < 0)
                    return;

                continue;
            }

            if ((mAudioInfo.streamIndex == -1) && (codecParams->codec_type == AVMEDIA_TYPE_AUDIO))
            {
                mAudioInfo.streamIndex = i;

                mAudioInfo.codecContext = avcodec_alloc_context3(codec);
                if (!mAudioInfo.codecContext)
                    continue;

                if (avcodec_parameters_to_context(mAudioInfo.codecContext, codecParams) < 0)
                    return;

                if (avcodec_open2(mAudioInfo.codecContext, codec, nullptr) < 0)
                    return;

                continue;
            }
        }

        // always needs video
        if (!mVideoInfo.codecContext)
            return;

        // to read data from the file and provide to the codec
        mVideoInfo.packet = av_packet_alloc();

        // random stuff
        mVideoInfo.avgFrameRate = av_q2d(mVideoInfo.stream->avg_frame_rate);
        mVideoInfo.pixelConvertContext =
          sws_getContext(mVideoInfo.codecContext->width, mVideoInfo.codecContext->height, mVideoInfo.codecContext->pix_fmt, mVideoInfo.codecContext->width,
                         mVideoInfo.codecContext->height, frameTargetPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

        // this is the frame where the video is decoded to
        mVideoInfo.videoFrameOriginal = av_frame_alloc();
        if (!mVideoInfo.videoFrameOriginal)
            return;

        // but also need a buffer where the video is converted to the target format
        int frameSize = av_image_get_buffer_size(frameTargetPixelFormat, mVideoInfo.codecContext->width, mVideoInfo.codecContext->height, 1);
        if (frameSize < 0)
            return;

        mVideoInfo.frameBufferSize = static_cast<size_t>(frameSize);

        // create a bunch of temp frames
        size_t skipQueue = 0;
        for (size_t i = 0; i < mVideoQueueMax; i++)
        {
            auto& targetFrame = mVideoQueue[i - skipQueue];

            targetFrame.avFrame = av_frame_alloc();
            if (!targetFrame.avFrame)
            {
                skipQueue++;
                continue;
            }

            targetFrame.frameData = (uint8_t*)av_malloc(mVideoInfo.frameBufferSize * sizeof(uint8_t));
            if (!targetFrame.frameData)
            {
                av_frame_free(&targetFrame.avFrame);

                skipQueue++;
                continue;
            }

            av_image_fill_arrays(targetFrame.avFrame->data, targetFrame.avFrame->linesize, targetFrame.frameData, frameTargetPixelFormat, mVideoInfo.codecContext->width,
                                 mVideoInfo.codecContext->height, 1);
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
        mVideoQueue.release();

        av_packet_free(&mVideoInfo.packet);
        sws_freeContext(mVideoInfo.pixelConvertContext);

        if (mVideoInfo.videoFrameOriginal)
            av_frame_free(&mVideoInfo.videoFrameOriginal);

        if (mAudioInfo.codecContext)
            avcodec_free_context(&mAudioInfo.codecContext);
        if (mVideoInfo.codecContext)
            avcodec_free_context(&mVideoInfo.codecContext);

        if (mFormatContext)
            avformat_close_input(&mFormatContext);
    }

    void VideoStream::process()
    {
        readAVFrame();
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
            // seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, mFormatContext->streams[streamIndex]->time_base);

            AVRational aux = {1, AV_TIME_BASE};
            seekTarget = av_rescale_q(0, aux, mFormatContext->streams[streamIndex]->time_base);
        }

        if (av_seek_frame(mFormatContext, streamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            return false;

        avcodec_flush_buffers(mVideoInfo.codecContext);
        if (mAudioInfo.codecContext)
            avcodec_flush_buffers(mAudioInfo.codecContext);

        mVideoQueueActive = 0;

        readAVFrame();

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
            // seekTarget = av_rescale_q(seekTarget, AV_TIME_BASE_Q, mFormatContext->streams[streamIndex]->time_base);

            AVRational aux = {1, AV_TIME_BASE};
            seekTarget = av_rescale_q(seekTarget, aux, mFormatContext->streams[streamIndex]->time_base);
        }

        auto lastFrame = getLatestFrame();

        if (av_seek_frame(mFormatContext, streamIndex, seekTarget, ((lastFrame != nullptr) && (lastFrame->framePTS > seekTarget)) ? AVSEEK_FLAG_BACKWARD : 0) < 0)
            return false;

        avcodec_flush_buffers(mVideoInfo.codecContext);
        if (mAudioInfo.codecContext)
            avcodec_flush_buffers(mAudioInfo.codecContext);

        mVideoQueueActive = 0;

        readAVFrame();

        if (mVideoQueueActive > 0)
            mTimerInfo.timer.setS(mVideoQueue[0].frameStart);
        else
            mTimerInfo.timer.setS(seconds);
        mTimerInfo.timestampS = mTimerInfo.timer.getTimeS();

        return true;
    }

    const void* VideoStream::getFrame(bool& clockIsBehind, int64_t& frameID, double& frameDurationS)
    {
        clockIsBehind = false;
        frameID = -1;
        frameDurationS = 0.0;

        mTimerInfo.timestampS = mTimerInfo.timer.getTimeS();

        while (true)
        {
            if (mVideoQueueActive <= 0)
            {
                readAVFrame();

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
                auto& targetFrame = mVideoQueue[0];

                frameID = targetFrame.framePTS;
                frameDurationS = targetFrame.frameEnd - mTimerInfo.timer.getTimeS();

                return targetFrame.frameData;
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

    double VideoStream::getFrameDuration(const int64_t& frameID) const
    {
        if (mVideoQueueActive <= 0)
            return 0.0;

        auto& targetFrame = mVideoQueue[0];
        if (targetFrame.framePTS != frameID)
            return 0.0;

        return (targetFrame.frameEnd - mTimerInfo.timer.getTimeS());
    }

    bool VideoStream::isValid() const
    {
        return (mVideoInfo.videoFrameOriginal && (mVideoQueueMax > 0));
    }

    bool VideoStream::hasAudio() const
    {
        return (mAudioInfo.codecContext != nullptr);
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

    void VideoStream::getVideoDims(size_t& videoWidth, size_t& videoHeight) const
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
            return hr::Rectangle<int>::zero();

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

            return hr::Rectangle<int>(hr::Math::ftoi((static_cast<float>(winWidth) - videoNewWidth) * 0.5f), 0, hr::Math::ftoi(videoNewWidth), winHeight);
        }
        else
        {
            auto videoNewHeight = static_cast<float>(winWidth) / videoScale;

            return hr::Rectangle<int>(0, hr::Math::ftoi((static_cast<float>(winHeight) - videoNewHeight) * 0.5f), winWidth, hr::Math::ftoi(videoNewHeight));
        }
    }

    hr::Rectangle<int> VideoStream::getVideoRectCenter(float scale, size_t winWidth, size_t winHeight) const
    {
        if (!mVideoInfo.codecContext)
            return hr::Rectangle<int>::zero();

        size_t videoWidth, videoHeight;
        getVideoDims(videoWidth, videoHeight);

        auto videoScale = static_cast<float>(videoWidth) / static_cast<float>(videoHeight);
        auto videoAspectRatio = getVideoFrameAspectRatio();
        if (videoAspectRatio > 0.0)
            videoScale *= videoAspectRatio;

        videoWidth = hr::Math::ftoi(static_cast<float>(videoWidth) * scale);
        videoHeight = hr::Math::ftoi(static_cast<float>(videoWidth) / videoScale);

        return hr::Rectangle<int>(hr::Math::ftoi(static_cast<float>(winWidth - videoWidth) * 0.5f), hr::Math::ftoi(static_cast<float>(winHeight - videoHeight) * 0.5f), videoWidth,
                                  videoHeight);
    }
}
