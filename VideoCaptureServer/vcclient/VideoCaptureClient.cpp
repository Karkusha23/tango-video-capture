#include "VideoCaptureClient.h"

namespace vc
{
	const char* IMAGE_WINDOW_NAME = "Image";
	const char* IMAGE_PARAMETERS_WINDOW_NAME = "Image parameters";

	cv::Scalar TEXT_COLOR = cv::Scalar(0, 255, 0);
	cv::Scalar RULER_COLOR = cv::Scalar(255, 0, 0);

	bool operator==(const Ruler& ruler1, const Ruler& ruler2)
	{
		return ruler1.start == ruler2.start && ruler1.end == ruler2.end && ruler1.length == ruler2.length;
	}

	double distance(const cv::Point& point1, const cv::Point& point2)
	{
		return std::sqrt((point1.x - point2.x) * (point1.x - point2.x) + (point1.y - point2.y) * (point1.y - point2.y));
	}

	VideoCaptureDevice::VideoCaptureDevice(const char* device_name, const char* playlist_path, const char* playlist_url, int framerate, bool to_show_ui) : to_show_ui_(to_show_ui)
	{
		std::cout << "Entering VideoCaptureDevice constructor" << std::endl;

		device_ = new Tango::DeviceProxy(device_name);

		std::cout << "Connected to device " << device_name << std::endl;

		cam_mode_ = get_device_camera_mode_();

		if (cam_mode_ == CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		threshold_ = threshold_prev_ = device_->read_attribute("Threshold").UShortSeq[0];

		ruler_ = ruler_prev_ = *reinterpret_cast<vc::Ruler*>(device_->read_attribute("Ruler").EncodedSeq[0].encoded_data.NP_data());
		ruler_length_ = (int)ruler_.length;

		width_ = get_device_int_property("Width");
		height_ = get_device_int_property("Height");

		mouse_flipflop_ = false;

		if (to_show_ui_)
		{
			cv::namedWindow(IMAGE_WINDOW_NAME);
			cv::setMouseCallback(IMAGE_WINDOW_NAME, image_mouse_callback, this);

			cv::namedWindow(IMAGE_PARAMETERS_WINDOW_NAME);
			cv::resizeWindow(IMAGE_PARAMETERS_WINDOW_NAME, 600, 300);
			cv::createTrackbar("Threshold", IMAGE_PARAMETERS_WINDOW_NAME, &threshold_, 100);
			cv::createTrackbar("Ruler Start X", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.start.x, width_);
			cv::createTrackbar("Ruler Start Y", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.start.y, height_);
			cv::createTrackbar("Ruler End   X", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.end.x, width_);
			cv::createTrackbar("Ruler End   Y", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.end.y, height_);
			cv::createTrackbar("Ruler length", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_length_, 1000);
		}

		image_ = cv::Mat(height_, width_, CV_8UC3);

		video_encoder_ = new VideoEncoderThread(playlist_path, playlist_url, width_, height_, framerate);

		std::cout << "VideoCaptureClient initialisation complete" << std::endl;
	}

	VideoCaptureDevice::~VideoCaptureDevice()
	{
		std::cout << "Entering VideoCaptureClient destructor" << std::endl;

		if (to_show_ui_)
		{
			cv::destroyWindow(IMAGE_WINDOW_NAME);
			cv::destroyWindow(IMAGE_PARAMETERS_WINDOW_NAME);
		}

		delete device_;
		delete video_encoder_;

		std::cout << "VideoCaptureClient destruction complete" << std::endl;
	}

	Tango::DeviceProxy& VideoCaptureDevice::device()
	{
		return *device_;
	}

	cv::Mat VideoCaptureDevice::image()
	{
		std::lock_guard<std::mutex> lock(image_lock_);
		cv::Mat result = image_.clone();
		return result;
	}

	std::vector<unsigned char> VideoCaptureDevice::jpg()
	{
		std::lock_guard<std::mutex> lock(image_lock_);
		return jpg_;
	}

	unsigned char* VideoCaptureDevice::image_data()
	{
		std::lock_guard<std::mutex> lock(image_lock_);
		uint8_t* data = new uint8_t[image_.total() * image_.elemSize()];
		std::memcpy(data, image_.data, image_.total() * image_.elemSize() * sizeof(uint8_t));
		return data;
	}

	int VideoCaptureDevice::cam_width() const
	{
		return width_;
	}

	int VideoCaptureDevice::cam_height() const
	{
		return height_;
	}

	void VideoCaptureDevice::print_device_info(std::ostream& out)
	{
		out << "Ping time: " << device_->ping() << std::endl;
		out << "Device name: " << device_->name() << std::endl;

		std::vector<std::string> device_properties;
		device_->get_property_list("*", device_properties);
		out << "Device Properties:" << std::endl;
		for (std::string& device_property : device_properties)
		{
			out << '\t' << device_property << ": ";
			Tango::DbData property_data;
			device_->get_property(device_property, property_data);
			for (Tango::DbDatum& data : property_data)
			{
				std::string outstring;
				data >> outstring;
				out << outstring << ' ';
			}
			out << std::endl;
		}
	}

	int VideoCaptureDevice::get_device_int_property(const std::string& name)
	{
		Tango::DbData* val_db = new Tango::DbData;

		std::string name_str(name);

		device_->get_property(name_str, *val_db);
		std::string string_val;
		(*val_db)[0] >> string_val;

		return std::stoi(string_val);
	}

	void VideoCaptureDevice::event_on_Jpeg_change(Tango::EventData* event_data)
	{
		std::lock_guard<std::mutex> lock(image_lock_);

		Tango::DeviceAttribute jpegAttr = device_->read_attribute("Jpeg");

		jpg_.resize(jpegAttr.EncodedSeq[0].encoded_data.length() / sizeof(unsigned char));
		std::memcpy(jpg_.data(), jpegAttr.EncodedSeq[0].encoded_data.NP_data(), jpegAttr.EncodedSeq[0].encoded_data.length() * sizeof(unsigned char));

		image_ = cv::imdecode(jpg_, cv::IMREAD_ANYCOLOR);

		if (cam_mode_ == CameraMode::Grayscale)
		{
			cv::Mat image_conv;
			cv::cvtColor(image_, image_conv, cv::COLOR_GRAY2RGB);
			image_ = image_conv;
		}

		Tango::DeviceAttribute contourAttr = device_->read_attribute("ContourInfo");

		vc::ContourInfo* contours = reinterpret_cast<vc::ContourInfo*>(contourAttr.EncodedSeq[0].encoded_data.NP_data());
		int contour_count = contourAttr.EncodedSeq[0].encoded_data.length() / sizeof(vc::ContourInfo);

		for (int i = 0; i < contour_count; ++i)
		{
			std::string text;

			if (contours[i].area_abs > 0.0)
			{
				text += "Area:" + std::to_string(contours[i].area_abs);
			}
			else
			{
				text += "Area(px):" + std::to_string(contours[i].area_rel);
			}

			text += "; ";

			if (contours[i].diameter_abs > 0.0)
			{
				text += "Diameter:" + std::to_string(contours[i].diameter_abs);
			}
			else
			{
				text += "Diameter(px):" + std::to_string(contours[i].diameter_rel);
			}

			cv::rectangle(image_, contours[i].bound_rect.tl(), contours[i].bound_rect.br(), TEXT_COLOR, 5);
			cv::circle(image_, contours[i].center_mass, 3, TEXT_COLOR, -1);
			cv::putText(image_, text, { contours[i].bound_rect.x, contours[i].bound_rect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
		}

		cv::line(image_, ruler_.start, ruler_.end, RULER_COLOR, 2);

		video_encoder_->writeFrame(image_);

		if (to_show_ui_)
		{
			cv::imshow(IMAGE_WINDOW_NAME, image_);
		}
	}

	void VideoCaptureDevice::update()
	{
		update_threshold_value_();
		update_ruler_();

		/*{
			std::lock_guard<std::mutex> lock(image_lock_);
			video_encoder_->writeFrame(image_);
		}*/
	}

	void VideoCaptureDevice::set_ruler_point_to(const cv::Point& point)
	{
		cv::Point& ruler_point = distance(point, ruler_.start) < distance(point, ruler_.end) ? ruler_.start : ruler_.end;

		//cv::Point& ruler_point = mouse_flipflop_ ? ruler_.start : ruler_.end;
		//mouse_flipflop_ = !mouse_flipflop_;

		ruler_point = point;
	}

	CameraMode VideoCaptureDevice::get_device_camera_mode_()
	{
		Tango::DbData* mode_db = new Tango::DbData;
		std::string mode_str = "Mode";
		std::string mode;

		device_->get_property(mode_str, *mode_db);
		(*mode_db)[0] >> mode;

		if (mode == "RGB" || mode == "rgb")
		{
			return CameraMode::RGB;
		}
		if (mode == "BGR" || mode == "bgr")
		{
			return CameraMode::BGR;
		}
		if (mode == "Grayscale" || mode == "grayscale")
		{
			return CameraMode::Grayscale;
		}

		return CameraMode::None;
	}

	void VideoCaptureDevice::update_threshold_value_()
	{
		if (threshold_ == threshold_prev_)
		{
			return;
		}

		threshold_prev_ = threshold_;

		std::string threshold_str = "Threshold";

		Tango::DeviceAttribute threshold_write(threshold_str, Tango::DevUShort(std::max(0, std::min(100, threshold_))));
		device_->write_attribute(threshold_write);
	}

	void VideoCaptureDevice::update_ruler_()
	{
		ruler_.length = (double)ruler_length_;

		if (ruler_ == ruler_prev_)
		{
			return;
		}

		ruler_prev_ = ruler_;

		Tango::DevEncoded ruler_encoded;
		ruler_encoded.encoded_data.length(sizeof(Ruler));
		std::memcpy(ruler_encoded.encoded_data.NP_data(), &ruler_, sizeof(Ruler));

		std::string ruler_str = "Ruler";

		Tango::DeviceAttribute ruler_write(ruler_str, ruler_encoded);
		device_->write_attribute(ruler_write);
	}

	void image_mouse_callback(int event, int x, int y, int flags, void* param)
	{
		if (event & cv::EVENT_LBUTTONDOWN)
		{
			reinterpret_cast<VideoCaptureDevice*>(param)->set_ruler_point_to(cv::Point(x, y));
		}
	}
}