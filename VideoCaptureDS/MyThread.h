#ifndef MyThreadIncluded
#define MyThreadIncluded

#include <string>

#include <tango.h>
#include "VideoCaptureDS.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace VideoCaptureDS_ns
{
	class MyThread : public omni_thread, public Tango::LogAdapter
	{
	public:

		MyThread(class VideoCaptureDS* dev, int source, int width, int height, const std::string& mode, int quality);
		~MyThread();

		void* run_undetached(void*);
		void stop();
		void execute_capture(cv::Mat* image, Tango::EncodedAttribute* jpeg, int* status);

		bool is_executing() const;
		bool is_failed() const;

		void connect(int source, int width, int height, const std::string& mode, int quality);

	private:

		class VideoCaptureDS* device_;

		bool local_exit_;
		bool to_capture_;
		bool is_executing_;
		bool is_failed_;

		int* status_;

		cv::VideoCapture* cam_;

		cv::Mat* image_no_image;

		enum class CameraMode : unsigned char { None, RGB, BGR, Grayscale } mode_;
		int width_;
		int height_;
		double quality_;

		cv::Mat* image;
		Tango::EncodedAttribute* jpeg;
	};
}	//	End of namespace

#endif