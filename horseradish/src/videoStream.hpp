#pragma once

#include "common\Platform.hpp"
#include "common\Types.hpp"
#include "common\Timer.hpp"
#include "common\Image.hpp"
#include "common\Primitives2D.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class VideoStream
{
private:
	static bool avInitialized;

	AVFormatContext *formatContext;

	struct TimerInfo{
		HorseRadish::Timer timer;
		double timestampS;
	}timerInfo;

	struct VideoInfo{
		AVCodecContext *codecContext;
		AVCodec *codec;
		AVFrame *videoFrameOriginal;
		AVStream *stream;
		double avgFrameRate;
		SwsContext *pixelConvertContext;
		int streamIndex, frameBufferSize;
	}videoInfo;

	struct VideoFrame{
		AVFrame *avFrame;
		uint8_t *frameData;
		double frameStart, frameDuration, frameEnd;
		int64_t framePTS;
	}*videoQueue;
	int videoQueueMax, videoQueueActive;

	struct AudioInfo{
		AVCodecContext *codecContext;
		AVCodec *codec;
		int streamIndex;
	}audioInfo;

	void processAVFrame();
	void recycleVideoFrameQueue();
	VideoFrame* getLatestFrame() const;

public:
	explicit VideoStream(const unsigned int maxFramesQueue, const PixelFormat frameTargetPixelFormat, const char * const videoFilePath);
	~VideoStream();

	static void Initialize();

	void Process();

	bool GoToBeginning();
	bool GoToTime(const double seconds);

	const void* GetFrame(bool &clockIsBehind, HorseRadish::hInt64 &frameID, double &frameDurationS);
	double GetFrameDuration(const HorseRadish::hInt64 &frameID);

	bool IsValid() const;
	bool HasAudio() const;
	double GetTimeStampDelta();

	int GetVideoFrameDataSize() const;
	double GetVideoFrameAspectRatio() const;
	void GetVideoDims(int &videoWidth, int &videoHeight) const;
	void GetVideoRect(const int winWidth, const int winHeight, const bool maintainAspectRatio, HorseRadish::Primitives2D::Rectangle<int> &rect) const;
	void GetVideoRectCenter(const float scale, const int winWidth, const int winHeight, HorseRadish::Primitives2D::Rectangle<int> &rect) const;
};
