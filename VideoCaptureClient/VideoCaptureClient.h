#pragma once
#include <iostream>
#include <exception>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

namespace vc
{
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

		// Write threshold value from user trackbar to device attribute
		void update_threshold_value();

	private:

		Tango::DeviceProxy device_;

		CameraMode cam_mode_;

		// Contour threshold value that is changing from opencv user trackbar
		int threshold_;

		// Previous value of threshold that updates on update_threshold_value()
		int threshold_prev_;

		// Get device camera mode from database property
		CameraMode get_device_camera_mode_();

		// Decode Jpeg from given Jpeg attribute
		cv::Mat decode_jpeg_(Tango::DeviceAttribute& devAttr);
	};

	// Wrapper for Tango event callback
	class JpegCallBack : public Tango::CallBack
	{
		VideoCaptureDevice* dev_;
	public:
		JpegCallBack(VideoCaptureDevice* dev) : Tango::CallBack(), dev_(dev) {}
		void push_event(Tango::EventData* event_data) override { dev_->event_function_Jpeg(event_data); }
	};
}