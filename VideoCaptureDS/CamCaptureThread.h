#ifndef _MyThreadIncluded
#define _MyThreadIncluded

#include <string>
#include <atomic>
#include <queue>
#include <algorithm>

#include <tango.h>
#include "VideoCaptureDS.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <vc/camproc.h>

namespace VideoCaptureDS_ns
{
	// Thread for capturing and processing images from webcam
	class CamCaptureThread : public omni_thread, public Tango::LogAdapter
	{
	public:

		CamCaptureThread(class VideoCaptureDS* dev, int source, int width, int height);
		~CamCaptureThread();

		void* run_undetached(void*);
		void stop();
		void capture(cv::Mat* image, std::vector<unsigned char>* jpeg, std::vector<vc::ContourInfo>* contours, const vc::Ruler* ruler,
					 vc::CameraMode mode, double jpegQuality, int threshold, std::atomic_bool* status);

		bool is_failed() const;

		void connect(int source, int width, int height);

	private:

		void get_contours_(const cv::Mat& image_gray, std::vector<vc::ContourInfo>* contourInfo, const vc::Ruler* ruler, int threshold);

		cv::Point get_center_of_mass_(const std::vector<cv::Point>& contour);

		static bool is_ruler_valid_(const vc::Ruler* ruler);

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
			std::vector<unsigned char>* jpeg;
			std::vector<vc::ContourInfo>* contours;
			const vc::Ruler* ruler;
			vc::CameraMode mode;
			double jpegQuality;
			int threshold;
			std::atomic_bool* status;
		};

		std::queue<CaptureQuery> queue_;
		omni_mutex queue_mutex_;

		std::vector<std::vector<cv::Point>> contours_;
		std::vector<cv::Vec4i> hierarchy_;
		std::vector<int> jpeg_params_;
	};
}	//	End of namespace

#endif