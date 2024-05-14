#pragma once

#include <iostream>
#include <exception>
#include <math.h>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

namespace vc
{
	extern const char* IMAGE_WINDOW_NAME;
	extern const char* IMAGE_PARAMETERS_WINDOW_NAME;

	extern cv::Scalar TEXT_COLOR;
	extern cv::Scalar RULER_COLOR;

	bool operator==(const Ruler& ruler1, const Ruler& ruler2);
	double distance(const cv::Point& point1, const cv::Point& point2);

	// Wrapper for videocapture device proxy
	class VideoCaptureDevice
	{
	public:

		VideoCaptureDevice(const char* device_name);
		~VideoCaptureDevice();

		Tango::DeviceProxy& device();

		void print_device_info(std::ostream& out);
		int get_device_int_property(const std::string& name);

		void event_function_Jpeg(Tango::EventData* event_data);

		// Called once in every update_time milliseconds
		void update();

		void set_ruler_point_to(const cv::Point& point);

	private:

		Tango::DeviceProxy device_;

		CameraMode cam_mode_;

		// Contour threshold value that is changing from opencv user trackbar
		int threshold_;

		// Previous value of threshold that updates on update_threshold_value_()
		int threshold_prev_;

		// Ruler that is changing from opencv UI
		Ruler ruler_;

		// Previous value of threshold that updates on update_ruler_()
		Ruler ruler_prev_;

		// Ruler length that is corresponding to UI trackbar
		int ruler_length_;

		// Flipflop for setting ruler point with mouse
		bool mouse_flipflop_;

		// Get device camera mode from database property
		CameraMode get_device_camera_mode_();

		// Decode Jpeg from given Jpeg attribute
		cv::Mat decode_jpeg_(Tango::DeviceAttribute& devAttr);

		// Write threshold value from user trackbar to device attribute
		void update_threshold_value_();

		// Write ruler from UI to device attributes
		void update_ruler_();
	};

	void image_mouse_callback(int event, int x, int y, int flags, void* param);

	// Wrapper for Tango event callback
	class JpegCallBack : public Tango::CallBack
	{
		VideoCaptureDevice* dev_;
	public:
		JpegCallBack(VideoCaptureDevice* dev) : Tango::CallBack(), dev_(dev) {}
		void push_event(Tango::EventData* event_data) override { dev_->event_function_Jpeg(event_data); }
	};
}