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

	VideoCaptureDevice::VideoCaptureDevice(const std::string& device_name) : device_name_(device_name)
	{
		std::cout << "Entering VideoCaptureDevice constructor" << std::endl;

		device_ = new Tango::DeviceProxy(device_name_);

		std::cout << "Connected to device " << device_name_ << std::endl;

		cam_mode_ = TangoDBWrapper::get_device_camera_mode(device_);

		if (cam_mode_ == CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		params_.threshold = params_prev_.threshold = device_->read_attribute("Threshold").UShortSeq[0];
		params_.minContourArea = params_prev_.minContourArea = device_->read_attribute("MinContourArea").DoubleSeq[0];

		params_.ruler = params_prev_.ruler = *reinterpret_cast<vc::Ruler*>(device_->read_attribute("Ruler").EncodedSeq[0].encoded_data.NP_data());

		width_ = TangoDBWrapper::get_device_int_property(device_, "Width");
		height_ = TangoDBWrapper::get_device_int_property(device_, "Height");

		device_name_formatted_ = device_name;
		while (true)
		{
			auto pos = device_name_formatted_.find("/");
			if (pos == std::string::npos)
			{
				break;
			}
			device_name_formatted_.replace(pos, 1, "-");
		}

		std::cout << "VideoCaptureClient initialisation complete" << std::endl;
	}

	VideoCaptureDevice::~VideoCaptureDevice()
	{
		std::cout << "Entering VideoCaptureClient destructor" << std::endl;

		delete device_;

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

	std::string VideoCaptureDevice::deviceNameFormatted() const
	{
		return device_name_formatted_;
	}

	cv::Mat VideoCaptureDevice::image()
	{
		std::lock_guard<std::mutex> lock(image_lock_);
		return image_.clone();
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

	int VideoCaptureDevice::out_width(UIDisplayType display_type) const
	{
		switch (display_type)
		{
		case UIDisplayType::SidePanel:
			return width_ + 300;
		default:
			return width_;
		}
	}

	int VideoCaptureDevice::out_height(UIDisplayType display_type) const
	{
		switch (display_type)
		{
		case UIDisplayType::SidePanel:
			return height_ < 480 ? 480 : height_;
		default:
			return height_;
		}
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

	void VideoCaptureDevice::event_on_Jpeg_change(Tango::EventData* event_data)
	{
		std::lock_guard<std::mutex> lock(image_lock_);

		Tango::DeviceAttribute* jpegAttr = event_data->attr_value;

		jpg_.resize(jpegAttr->EncodedSeq[0].encoded_data.length() / sizeof(unsigned char));
		std::memcpy(jpg_.data(), jpegAttr->EncodedSeq[0].encoded_data.NP_data(), jpegAttr->EncodedSeq[0].encoded_data.length() * sizeof(unsigned char));

		image_ = cv::imdecode(jpg_, cv::IMREAD_COLOR);

		{
			cv::Mat image_conv;
			cv::cvtColor(image_, image_conv, cv::COLOR_BGR2RGB);
			image_ = std::move(image_conv);
		}

		contourAttr_ = device_->read_attribute("ContourInfo");

		contours_ = reinterpret_cast<vc::ContourInfo*>(contourAttr_.EncodedSeq[0].encoded_data.NP_data());
		contour_count_ = contourAttr_.EncodedSeq[0].encoded_data.length() / sizeof(vc::ContourInfo);

		write_to_encoders_(UIDisplayType::None, image_);
		put_frame_ui_(UIDisplayType::NoText);
		write_to_encoders_(UIDisplayType::NoText, image_);
		put_frame_ui_(UIDisplayType::SidePanel);
		write_to_encoders_(UIDisplayType::SidePanel, image_pad_);
		put_frame_ui_(UIDisplayType::Regular);
		write_to_encoders_(UIDisplayType::Regular, image_);
	}

	void VideoCaptureDevice::put_frame_ui_(UIDisplayType display_type)
	{
		if (display_type == UIDisplayType::None)
		{
			return;
		}

		if (display_type == UIDisplayType::SidePanel)
		{
			cv::copyMakeBorder(image_, image_pad_, 0, out_height(UIDisplayType::SidePanel) - height_, 0, out_width(UIDisplayType::SidePanel) - width_, cv::BORDER_CONSTANT);
		}

		if (display_type == UIDisplayType::NoText)
		{
			for (int i = 0; i < contour_count_; ++i)
			{
				cv::rectangle(image_, contours_[i].bound_rect.tl(), contours_[i].bound_rect.br(), TEXT_COLOR, 5);
				cv::circle(image_, contours_[i].center_mass, 3, TEXT_COLOR, -1);
			}
			cv::line(image_, params_.ruler.start, params_.ruler.end, RULER_COLOR, 2);
		}
		else
		{

			for (int i = 0; i < contour_count_; ++i)
			{
				std::string text;

				if (contours_[i].area_abs > 0.0)
				{
					text += "Area:" + std::to_string((int)contours_[i].area_abs);
				}
				else
				{
					text += "Area(px):" + std::to_string((int)contours_[i].area_rel);
				}

				text += "; ";

				if (contours_[i].diameter_abs > 0.0)
				{
					text += "Diameter:" + std::to_string((int)contours_[i].diameter_abs);
				}
				else
				{
					text += "Diameter(px):" + std::to_string((int)contours_[i].diameter_rel);
				}

				switch (display_type)
				{
				case UIDisplayType::Regular:
					cv::putText(image_, text, { contours_[i].bound_rect.x, contours_[i].bound_rect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
					break;
				case UIDisplayType::SidePanel:
					cv::putText(image_pad_, text, { width_ + 2, 10 * (i + 1) }, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
					break;
				default:
					break;
				}
			}
		}
	}

	void VideoCaptureDevice::write_to_encoders_(UIDisplayType display_type, const cv::Mat& image)
	{
		std::lock_guard<std::mutex> lock(encoders_lock_);
		auto range = encoder_types_.equal_range(display_type);
		for (auto it = range.first; it != range.second; ++it)
		{
			it->second->writeFrame(image);
		}
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

	std::pair<int, std::string> VideoCaptureDevice::add_encoder(const std::string& playlist_base_path, const std::string& playlist_base_url, bool isRecording, UIDisplayType display_type)
	{
		std::cout << "Adding new encoder" << std::endl;

		std::lock_guard<std::mutex> lock(encoders_lock_);
		int id = -1;
		for (const auto& it : encoders_)
		{
			if (it.first - id > 1)
			{
				break;
			}
			id = it.first;
		}
		++id;
		std::string suffix = device_name_formatted_ + "-" + std::to_string(id);
		if (isRecording)
		{
			suffix += "-" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
		}
		display_type = isRecording ? UIDisplayType::NoText : display_type;
		auto encoder = std::make_shared<VideoEncoderThread>(playlist_base_path + "\\" + suffix, playlist_base_url + suffix + "/", out_width(display_type), out_height(display_type), isRecording);
		encoders_.insert({ id, Encoder({ encoder , display_type, isRecording })});
		encoder_types_.insert({ display_type, encoder });

		std::cout << "Created new encoder with name " << suffix << std::endl;

		return { id, suffix };
	}

	bool VideoCaptureDevice::remove_encoder(int id)
	{
		std::cout << "Removing encoder" << std::endl;

		std::lock_guard<std::mutex> lock(encoders_lock_);
		if (!encoders_.count(id))
		{
			return false;
		}

		{
			std::shared_ptr<VideoEncoderThread> encoder = encoders_[id].encoder;
			for (auto it = encoder_types_.begin(); it != encoder_types_.end(); ++it)
			{
				if (it->second == encoder)
				{
					encoder_types_.erase(it);
					break;
				}
			}
			encoders_.erase(id);
		}

		std::cout << "Encoder removed" << std::endl;

		return true;
	}

	int VideoCaptureDevice::encoder_count()
	{
		std::lock_guard<std::mutex> lock(encoders_lock_);
		return encoders_.size();
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

	Tango::DbData* TangoDBWrapper::data_(new Tango::DbData);
	std::mutex TangoDBWrapper::data_lock_;

	int TangoDBWrapper::get_device_int_property(Tango::DeviceProxy* device, const std::string& property_name)
	{
		std::lock_guard<std::mutex> lock(data_lock_);

		std::string name_str(property_name);

		device->get_property(name_str, *data_);
		std::string string_val;
		(*data_)[0] >> string_val;

		return std::stoi(string_val);
	}

	CameraMode TangoDBWrapper::get_device_camera_mode(Tango::DeviceProxy* device)
	{
		std::lock_guard<std::mutex> lock(data_lock_);

		std::string mode_str = "Mode";
		std::string mode;

		device->get_property(mode_str, *data_);
		(*data_)[0] >> mode;

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
}