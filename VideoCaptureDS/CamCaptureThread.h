#ifndef MyThreadIncluded
#define MyThreadIncluded

#include <string>
#include <atomic>
#include <queue>

#include <tango.h>
#include "VideoCaptureDS.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace VideoCaptureDS_ns
{
	enum class CameraMode : unsigned char { RGB, BGR, Grayscale };

	class CamCaptureThread : public omni_thread, public Tango::LogAdapter
	{
	public:

		CamCaptureThread(class VideoCaptureDS* dev, int source, int width, int height);
		~CamCaptureThread();

		void* run_undetached(void*);
		void stop();
		void execute_capture(cv::Mat* image, Tango::EncodedAttribute* jpeg, CameraMode mode, double jpegQuality, bool* status);

		bool is_failed() const;

		void connect(int source, int width, int height);

	private:

		class VideoCaptureDS* device_;

		std::atomic_bool local_exit_;
		std::atomic_bool is_failed_;

		cv::VideoCapture* cam_;
		cv::Mat* image_no_image;

		omni_mutex cam_mutex_;

		int width_;
		int height_;

		struct CaptureQuery
		{
			cv::Mat* image;
			Tango::EncodedAttribute* jpeg;
			CameraMode mode;
			double jpegQuality;
			bool* status;
		};

		std::queue<CaptureQuery> queue_;
		omni_mutex queue_mutex_;
	};
}	//	End of namespace

#endif