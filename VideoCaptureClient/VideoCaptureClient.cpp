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

	VideoCaptureDevice::VideoCaptureDevice(const char* device_name) : device_(device_name)
	{
		cam_mode_ = get_device_camera_mode_();

		if (cam_mode_ == CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		threshold_ = threshold_prev_ = device_.read_attribute("Threshold").UShortSeq[0];

		ruler_ = ruler_prev_ = *reinterpret_cast<vc::Ruler*>(device_.read_attribute("Ruler").EncodedSeq[0].encoded_data.NP_data());
		ruler_length_ = (int)ruler_.length;

		int width = get_device_int_property("Width");
		int height = get_device_int_property("Height");

		mouse_flipflop_ = false;

		cv::namedWindow(IMAGE_WINDOW_NAME);
		cv::setMouseCallback(IMAGE_WINDOW_NAME, image_mouse_callback, this);

		cv::namedWindow(IMAGE_PARAMETERS_WINDOW_NAME);
		cv::resizeWindow(IMAGE_PARAMETERS_WINDOW_NAME, 600, 300);
		cv::createTrackbar("Threshold", IMAGE_PARAMETERS_WINDOW_NAME, &threshold_, 100);
		cv::createTrackbar("Ruler Start X", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.start.x, width);
		cv::createTrackbar("Ruler Start Y", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.start.y, height);
		cv::createTrackbar("Ruler End   X", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.end.x, width);
		cv::createTrackbar("Ruler End   Y", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_.end.y, height);
		cv::createTrackbar("Ruler length", IMAGE_PARAMETERS_WINDOW_NAME, &ruler_length_, 1000);
	}

	VideoCaptureDevice::~VideoCaptureDevice()
	{
		cv::destroyWindow(IMAGE_WINDOW_NAME);
		cv::destroyWindow(IMAGE_PARAMETERS_WINDOW_NAME);
	}

	Tango::DeviceProxy& VideoCaptureDevice::device()
	{
		return device_;
	}

	void VideoCaptureDevice::print_device_info(std::ostream& out)
	{
		out << "Ping time: " << device_.ping() << std::endl;
		out << "Device name: " << device_.name() << std::endl;

		std::vector<std::string> device_properties;
		device_.get_property_list("*", device_properties);
		out << "Device Properties:" << std::endl;
		for (std::string& device_property : device_properties)
		{
			out << '\t' << device_property << ": ";
			Tango::DbData property_data;
			device_.get_property(device_property, property_data);
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
		Tango::DbData val_db;
		device_.get_property(std::string(name), val_db);
		std::string string_val;
		val_db[0] >> string_val;

		return std::stoi(string_val);
	}

	void VideoCaptureDevice::event_function_Jpeg(Tango::EventData* event_data)
	{
		cv::Mat image = decode_jpeg_(device_.read_attribute("Jpeg"));

		Tango::DeviceAttribute contourAttr = device_.read_attribute("ContourInfo");

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

			cv::rectangle(image, contours[i].bound_rect.tl(), contours[i].bound_rect.br(), TEXT_COLOR, 5);
			cv::circle(image, contours[i].center_mass, 3, TEXT_COLOR, -1);
			cv::putText(image, text, { contours[i].bound_rect.x, contours[i].bound_rect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, TEXT_COLOR);
		}

		cv::line(image, ruler_.start, ruler_.end, RULER_COLOR, 2);

		cv::imshow(IMAGE_WINDOW_NAME, image);
	}

	void VideoCaptureDevice::update()
	{
		update_threshold_value_();
		update_ruler_();
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
		Tango::DbData mode_db;
		device_.get_property(std::string("Mode"), mode_db);
		std::string mode;
		mode_db[0] >> mode;

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

	cv::Mat VideoCaptureDevice::decode_jpeg_(Tango::DeviceAttribute& devAttr)
	{
		unsigned char* image_byte = nullptr;
		cv::Mat image_converted;

		try
		{
			int width, height;

			Tango::EncodedAttribute enAttr;

			cv::Mat image_mat;

			switch (cam_mode_)
			{
			case vc::CameraMode::RGB:
				enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_RGBA2BGR);
				break;
			case vc::CameraMode::BGR:
				enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_BGRA2BGR);
				break;
			case vc::CameraMode::Grayscale:
				enAttr.decode_gray8(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC1, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_GRAY2BGR);
				break;
			default:
				break;
			}
		}
		catch (...)
		{
			delete[] image_byte;
			throw;
		}

		delete[] image_byte;

		return image_converted;
	}

	void VideoCaptureDevice::update_threshold_value_()
	{
		if (threshold_ == threshold_prev_)
		{
			return;
		}

		threshold_prev_ = threshold_;
		Tango::DeviceAttribute threshold_write(std::string("Threshold"), Tango::DevUShort(std::max(0, std::min(100, threshold_))));
		device_.write_attribute(threshold_write);
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

		Tango::DeviceAttribute ruler_write(std::string("Ruler"), ruler_encoded);
		device_.write_attribute(ruler_write);
	}

	void image_mouse_callback(int event, int x, int y, int flags, void* param)
	{
		if (event & cv::EVENT_LBUTTONDOWN)
		{
			reinterpret_cast<VideoCaptureDevice*>(param)->set_ruler_point_to(cv::Point(x, y));
		}
	}
}