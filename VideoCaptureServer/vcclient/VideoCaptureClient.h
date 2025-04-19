#ifndef VideoCaptureClient_included
#define VideoCaptureClient_included

#include <iostream>
#include <vector>
#include <exception>
#include <math.h>
#include <map>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

#include "VideoEncoderThread.h"
#include "VideoInfoWriter.h"

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
			NoText, // Adding bounding rects and mass centers for contours
			SidePanel, // Same as regular, but frames are extended by width and have side panel on which additional info is displayed
			Regular // Adding bounding rects and mass centers for contours, other info about contours is displayed above said rects
		};

		// device_name - name of device in Tango system
		// playlist_path - full path of .m3u8 that will be generated along with .ts files in the same folder
		// playlist_url - url that will be set in .m3u8 playlist. Media player will then get .ts files with this url
		// display_type - type of UI displayed on video frames
		VideoCaptureDevice(const std::string& device_name);
		~VideoCaptureDevice();

		Tango::DeviceProxy& device();
		std::string deviceName() const;
		std::string deviceNameFormatted() const;
		cv::Mat image();
		unsigned char* image_data();
		std::vector<unsigned char> jpg();

		int cam_width() const;
		int cam_height() const;

		int out_width(UIDisplayType display_type) const;
		int out_height(UIDisplayType display_type) const;

		void print_device_info(std::ostream& out);

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

		std::pair<int, std::string> add_encoder(const std::string& playlist_base_path, const std::string& playlist_base_url, bool isRecording = false, UIDisplayType display_type = UIDisplayType::NoText);
		bool remove_encoder(int id);
		int encoder_count();

	private:

		Tango::DeviceProxy* device_;
		std::string device_name_;
		std::string device_name_formatted_;

		cv::Mat image_;
		cv::Mat image_pad_;
		std::vector<unsigned char> jpg_;

		Tango::DeviceAttribute contourAttr_;
		std::vector<vc::ContourInfo> contours_;
		int contour_count_;

		std::mutex image_lock_;
		std::mutex params_lock_;
		std::mutex encoders_lock_;

		CameraMode cam_mode_;
		int width_;
		int height_;

		Params params_;
		Params params_prev_;

		// Put ui on a frame according to display type
		// Called several time consequently for every display type during execution of event_on_Jpeg_change() method
		void put_frame_ui_(UIDisplayType display_type);

		// Write threshold value from user trackbar to device attribute
		void update_threshold_value_();

		// Write minimal contour area value from user trackbar to device attribute
		void update_MinContourArea_value_();

		// Write ruler from UI to device attributes
		void update_ruler_();

		void write_to_encoders_(UIDisplayType display_type, const cv::Mat& image);
		void write_to_infowriter_(int id, int64_t pts);

		std::map<int, std::pair<std::shared_ptr<VideoEncoderThread>, UIDisplayType>> encoders_;
		std::multimap<UIDisplayType, std::pair<std::shared_ptr<VideoEncoderThread>, int>> encoder_types_;

		std::map<int, std::shared_ptr<VideoInfoWriter>> cinfo_writers_;
	};

	class JpegCallBack : public Tango::CallBack
	{
		std::shared_ptr<VideoCaptureDevice> dev_;
	public:
		JpegCallBack(const std::shared_ptr<VideoCaptureDevice>& dev) : Tango::CallBack(), dev_(dev) {}
		void push_event(Tango::EventData* event_data) override { dev_->event_on_Jpeg_change(event_data); }
	};

	class TangoDBWrapper
	{
		static Tango::DbData* data_;
		static std::mutex data_lock_;
	public:
		static int get_device_int_property(Tango::DeviceProxy* device, const std::string& property_name);
		static CameraMode get_device_camera_mode(Tango::DeviceProxy* device);
	};
}

#endif