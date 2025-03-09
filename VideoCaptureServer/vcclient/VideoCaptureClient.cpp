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

	VideoCaptureDevice::VideoCaptureDevice(const std::string& device_name, const std::string& playlist_path, const std::string& playlist_url, UIDisplayType display_type) :
		device_name_(device_name), display_type_(display_type)
	{
		std::cout << "Entering VideoCaptureDevice constructor" << std::endl;

		device_ = new Tango::DeviceProxy(device_name_);

		std::cout << "Connected to device " << device_name_ << std::endl;

		cam_mode_ = get_device_camera_mode_();

		if (cam_mode_ == CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		params_.threshold = params_prev_.threshold = device_->read_attribute("Threshold").UShortSeq[0];
		params_.minContourArea = params_prev_.minContourArea = device_->read_attribute("MinContourArea").DoubleSeq[0];

		params_.ruler = params_prev_.ruler = *reinterpret_cast<vc::Ruler*>(device_->read_attribute("Ruler").EncodedSeq[0].encoded_data.NP_data());

		width_ = get_device_int_property("Width");
		height_ = get_device_int_property("Height");

		if (display_type_ == UIDisplayType::SidePanel)
		{
			out_height_ = height_ >= 480 ? height_ : 480;
			out_width_ = width_ + 200;
		}
		else
		{
			out_width_ = width_;
			out_height_ = height_;
		}

		image_ = cv::Mat(out_height_, out_width_, CV_8UC3);

		video_encoder_ = new VideoEncoderThread(playlist_path, playlist_url, out_width_, out_height_);

		std::cout << "VideoCaptureClient initialisation complete" << std::endl;
	}

	VideoCaptureDevice::~VideoCaptureDevice()
	{
		std::cout << "Entering VideoCaptureClient destructor" << std::endl;

		delete device_;
		delete video_encoder_;

		std::cout << "VideoCaptureClient destruction complete" << std::endl;
	}

	Tango::DeviceProxy& VideoCaptureDevice::device()
	{
		return *device_;
	}

	std::string VideoCaptureDevice::deviceName() const
	{
		return device_name_;
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

	int VideoCaptureDevice::out_width() const
	{
		return out_width_;
	}

	int VideoCaptureDevice::out_height() const
	{
		return out_height_;
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

		image_ = cv::imdecode(jpg_, cv::IMREAD_COLOR);
		cv::Mat image_conv;
		cv::cvtColor(image_, image_conv, cv::COLOR_BGR2RGB);
		image_ = image_conv;

		put_ui_on_frame_();

		video_encoder_->writeFrame(image_);
	}

	void VideoCaptureDevice::put_ui_on_frame_()
	{
		if (display_type_ == UIDisplayType::None)
		{
			return;
		}

		if (display_type_ == UIDisplayType::SidePanel)
		{
			cv::Mat image_pad;
			cv::copyMakeBorder(image_, image_pad, 0, out_height_ - height_, 0, out_width_ - width_, cv::BORDER_CONSTANT);
			image_ = image_pad;
		}

		Tango::DeviceAttribute contourAttr = device_->read_attribute("ContourInfo");

		vc::ContourInfo* contours = reinterpret_cast<vc::ContourInfo*>(contourAttr.EncodedSeq[0].encoded_data.NP_data());
		int contour_count = contourAttr.EncodedSeq[0].encoded_data.length() / sizeof(vc::ContourInfo);

		for (int i = 0; i < contour_count; ++i)
		{
			std::string text;

			if (contours[i].area_abs > 0.0)
			{
				text += "Area:" + std::to_string((int)contours[i].area_abs);
			}
			else
			{
				text += "Area(px):" + std::to_string((int)contours[i].area_rel);
			}

			text += "; ";

			if (contours[i].diameter_abs > 0.0)
			{
				text += "Diameter:" + std::to_string((int)contours[i].diameter_abs);
			}
			else
			{
				text += "Diameter(px):" + std::to_string((int)contours[i].diameter_rel);
			}

			cv::rectangle(image_, contours[i].bound_rect.tl(), contours[i].bound_rect.br(), TEXT_COLOR, 5);
			cv::circle(image_, contours[i].center_mass, 3, TEXT_COLOR, -1);

			switch (display_type_)
			{
			case UIDisplayType::Regular:
				cv::putText(image_, text, { contours[i].bound_rect.x, contours[i].bound_rect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
				break;
			case UIDisplayType::SidePanel:
				cv::putText(image_, text, { width_ + 2, 10 * (i + 1)}, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
				break;
			default:
				break;
			}
		}

		cv::line(image_, params_.ruler.start, params_.ruler.end, RULER_COLOR, 2);
	}

	void VideoCaptureDevice::update()
	{
		std::lock_guard<std::mutex> lock(params_lock_);
		update_threshold_value_();
		update_MinContourArea_value_();
		update_ruler_();
	}

	VideoCaptureDevice::Params VideoCaptureDevice::get_params()
	{
		std::lock_guard<std::mutex> lock(params_lock_);
		return params_;
	}

	void VideoCaptureDevice::set_params(const VideoCaptureDevice::Params& params)
	{
		std::lock_guard<std::mutex> lock(params_lock_);
		params_ = params;
	}

	void VideoCaptureDevice::set_ruler_point_to(const cv::Point& point)
	{
		cv::Point& ruler_point = distance(point, params_.ruler.start) < distance(point, params_.ruler.end) ? params_.ruler.start : params_.ruler.end;
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
		if (params_.threshold == params_prev_.threshold)
		{
			return;
		}

		params_prev_.threshold = params_.threshold;

		std::string threshold_str = "Threshold";

		Tango::DeviceAttribute threshold_write(threshold_str, Tango::DevUShort(std::max(0, std::min(100, params_.threshold))));
		device_->write_attribute(threshold_write);
	}

	void VideoCaptureDevice::update_MinContourArea_value_()
	{
		if (params_.minContourArea == params_prev_.minContourArea)
		{
			return;
		}

		params_prev_.minContourArea = params_.minContourArea;

		std::string minContourArea_str = "MinContourArea";

		Tango::DeviceAttribute minContourArea_write(minContourArea_str, Tango::DevDouble(params_.minContourArea));
		device_->write_attribute(minContourArea_write);
	}

	void VideoCaptureDevice::update_ruler_()
	{
		if (params_.ruler == params_prev_.ruler)
		{
			return;
		}

		params_prev_.ruler = params_.ruler;

		Tango::DevEncoded ruler_encoded;
		ruler_encoded.encoded_data.length(sizeof(Ruler));
		std::memcpy(ruler_encoded.encoded_data.NP_data(), &params_.ruler, sizeof(Ruler));

		std::string ruler_str = "Ruler";

		Tango::DeviceAttribute ruler_write(ruler_str, ruler_encoded);
		device_->write_attribute(ruler_write);
	}
}