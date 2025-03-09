#ifndef VideoCaptureClient_included
#define VideoCaptureClient_included

#include <iostream>
#include <vector>
#include <exception>
#include <math.h>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

#include "VideoEncoderThread.h"

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

		enum class UIDisplayType 
		{
			None, // Encoding raw frames without any UI
			Regular, // Adding bounding rects and mass centers for contours, other info about contours is displayed above said rects
			SidePanel // Same as regular, but frames are extended by width and have side panel on which additional info is displayed
		};

		// device_name - name of device in Tango system
		// playlist_path - full path of .m3u8 that will be generated along with .ts files in the same folder
		// playlist_url - url that will be set in .m3u8 playlist. Media player will then get .ts files with this url
		// to_show_ui - to show ui on host pc
		VideoCaptureDevice(const char* device_name, const char* playlist_path, const char* playlist_url, UIDisplayType display_type);
		~VideoCaptureDevice();

		Tango::DeviceProxy& device();
		std::string deviceName() const;
		cv::Mat image();
		unsigned char* image_data();
		std::vector<unsigned char> jpg();

		int cam_width() const;
		int cam_height() const;

		void print_device_info(std::ostream& out);
		int get_device_int_property(const std::string& name);

		void event_on_Jpeg_change(Tango::EventData* event_data);

		// Called once in every update_time milliseconds
		void update();

		struct Params
		{
			Ruler ruler;
			int threshold;
			double minContourArea;
		};

		Params get_params();
		void set_params(const Params& params);

		void set_ruler_point_to(const cv::Point& point);

	private:

		Tango::DeviceProxy* device_;
		std::string device_name_;

		VideoEncoderThread* video_encoder_;

		cv::Mat image_;
		std::vector<unsigned char> jpg_;

		std::mutex image_lock_;
		std::mutex params_lock_;

		CameraMode cam_mode_;
		int width_;
		int height_;

		int out_width_;
		int out_height_;

		const UIDisplayType display_type_;

		Params params_;
		Params params_prev_;

		// Get device camera mode from database property
		CameraMode get_device_camera_mode_();

		void put_ui_on_frame_();

		// Write threshold value from user trackbar to device attribute
		void update_threshold_value_();

		// Write minimal contour area value from user trackbar to device attribute
		void update_MinContourArea_value_();

		// Write ruler from UI to device attributes
		void update_ruler_();
	};

	class JpegCallBack : public Tango::CallBack
	{
		VideoCaptureDevice* dev_;
	public:
		JpegCallBack(VideoCaptureDevice* dev) : Tango::CallBack(), dev_(dev) {}
		void push_event(Tango::EventData* event_data) override { dev_->event_on_Jpeg_change(event_data); }
	};

	//void image_mouse_callback(int event, int x, int y, int flags, void* param);
}

#endif