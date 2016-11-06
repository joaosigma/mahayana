#pragma once

#include "../common/types.hpp"
#include "../common/timer.hpp"
#include "../common/image.hpp"
#include "../common/primitives2D.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace hr { namespace misc
{
	class VideoStream
	{
	private:
		AVFormatContext *mFormatContext = nullptr;

		struct TimerInfo{
			hr::Timer timer;
			double timestampS = 0.0;
		}mTimerInfo;

		struct VideoInfo{
			AVCodecContext *codecContext = nullptr;
			AVCodec *codec = nullptr;
			AVFrame *videoFrameOriginal = nullptr;
			AVStream *stream = nullptr;
			double avgFrameRate = 0.0;
			SwsContext *pixelConvertContext = nullptr;
			int streamIndex = -1;
			size_t frameBufferSize = 0;
		}mVideoInfo;

		struct VideoFrame{
			AVFrame *avFrame = nullptr;
			uint8_t *frameData = nullptr;
			double frameStart = 0.0, frameDuration = 0.0, frameEnd = 0.0;
			int64_t framePTS = 0;
		}*mVideoQueue;
		size_t mVideoQueueMax = 0, mVideoQueueActive = 0;

		struct AudioInfo{
			AVCodecContext *codecContext = nullptr;
			AVCodec *codec = nullptr;
			int streamIndex = -1;
		}mAudioInfo;

		void processAVFrame();
		void recycleVideoFrameQueue();
		VideoFrame* getLatestFrame() const;

	public:
		explicit VideoStream(size_t maxFramesQueue, PixelFormat frameTargetPixelFormat, const char * const videoFilePath);
		~VideoStream();

		static void Initialize();

		void process();

		bool goToBeginning();
		bool goToTime(double seconds);

		const void* getFrame(bool &clockIsBehind, hr::hInt64 &frameID, double &frameDurationS);
		double getFrameDuration(const hr::hInt64 &frameID)  const;

		bool isValid() const;
		bool hasAudio() const;
		double getTimeStampDelta() const;

		size_t getVideoFrameDataSize() const;
		double getVideoFrameAspectRatio() const;
		void getVideoDims(size_t &videoWidth, size_t &videoHeight) const;
		hr::Rectangle<int> getVideoRect(size_t winWidth, size_t winHeight, bool maintainAspectRatio) const;
		hr::Rectangle<int> getVideoRectCenter(float scale, size_t winWidth, size_t winHeight) const;
	};

}}