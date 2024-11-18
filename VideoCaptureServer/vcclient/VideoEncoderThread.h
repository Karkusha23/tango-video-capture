#ifndef VideoEncoderThread_included
#define VideoEncoderThread_included

#include <opencv2/core.hpp>

#include <tango.h>

#include <string>
#include <iostream>
#include <queue>

#include <atomic>
#include <mutex>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

#include "../mythread/MyThread.h"

namespace vc
{
	class VideoEncoderThread : public MyThread
	{
	public:

		VideoEncoderThread(const std::string& playlist_path, const std::string& playlist_url, int cam_width, int cam_height, int framerate);
		virtual ~VideoEncoderThread();

		void writeFrame(const cv::Mat& image);

	private:

		std::string playlist_path_;
		std::string playlist_url_;
		int framerate_;
		int frame_count_;
		AVFormatContext* format_context_;
		const AVOutputFormat* output_format_;
		AVDictionary* options_;
		AVStream* stream_;
		const AVCodec* codec_;
		AVCodecContext* codec_context_;
		AVFrame* av_frame_;
		struct SwsContext* sws_context_;
		AVPacket packet_;

		struct WriteQuery
		{
			uint8_t* data;
			int rows;
			int cols;
		};

		std::queue<WriteQuery> queue_;
		std::mutex queue_lock_;

		void write_frame_from_queue_();
		void update() override;
	};
}

#endif