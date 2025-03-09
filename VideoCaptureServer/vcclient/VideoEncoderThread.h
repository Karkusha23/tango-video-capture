#ifndef VideoEncoderThread_included
#define VideoEncoderThread_included

#include <opencv2/core.hpp>

#include <tango.h>

#include <string>
#include <iostream>
#include <queue>
#include <chrono>

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

// Thread dedicated to encoding stream of OpenCV images to .m3u8 HLS playlist using FFMPEG

namespace vc
{
	class VideoEncoderThread : public MyThread
	{
	public:

		// playlist_path - full path of .m3u8 that will be generated along with .ts files in the same folder
		// playlist_url - url that will be set in .m3u8 playlist. Media player will then get .ts files with this url
		VideoEncoderThread(const std::string& playlist_path, const std::string& playlist_url, int cam_width, int cam_height);
		virtual ~VideoEncoderThread();

		// Write frame to the queue
		void writeFrame(const cv::Mat& image);

	private:

		const std::string playlist_path_;
		const std::string playlist_head_path_;
		const std::string playlist_url_;

		const int framerate_;
		int frame_count_;

		const AVOutputFormat* output_format_;
		const AVCodec* codec_;

		AVFormatContext* format_context_;
		AVDictionary* options_;
		AVStream* stream_;
		AVCodecContext* codec_context_;
		AVFrame* av_frame_;
		SwsContext* sws_context_;
		AVPacket packet_;

		bool wrote_first_frame_;
		std::chrono::high_resolution_clock::time_point first_frame_time_;

		struct WriteQuery
		{
			uint8_t* data;
			int rows;
			int cols;
			time_t pts;
		};

		std::queue<WriteQuery> queue_;
		std::mutex queue_lock_;

		// Get first image from queue and write it to playlist
		void write_frame_from_queue_();

		// Write frame from query
		void write_frame_from_(const WriteQuery& query);

		void update() override;
	};
}

#endif