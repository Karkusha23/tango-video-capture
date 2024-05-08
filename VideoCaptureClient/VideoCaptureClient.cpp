#include "VideoCaptureClient.h"

namespace vc
{
	VideoCaptureDevice::VideoCaptureDevice(const char* device_name) : device_(device_name)
	{
		cam_mode_ = get_device_camera_mode_();

		if (cam_mode_ == CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		threshold_ = threshold_prev_ = device_.read_attribute("Threshold").UShortSeq[0];

		cv::namedWindow("Threshold", (640, 200));
		cv::createTrackbar("Threshold", "Threshold", &threshold_, 100);
	}

	VideoCaptureDevice::~VideoCaptureDevice()
	{
		cv::destroyWindow("Image");
		cv::destroyWindow("Threshold");
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
			std::string text = "Area:" + std::to_string(contours[i].area) /* + "; Perimeter: " + std::to_string(contours[i].perimeter)*/;

			cv::rectangle(image, contours[i].boundRect.tl(), contours[i].boundRect.br(), cv::Scalar(0, 255, 0), 5);
			cv::putText(image, text, { contours[i].boundRect.x, contours[i].boundRect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0));
		}

		cv::imshow("Image", image);
	}

	void VideoCaptureDevice::update_threshold_value()
	{
		if (threshold_ != threshold_prev_)
		{
			threshold_prev_ = threshold_;
			Tango::DeviceAttribute threshold_write(std::string("Threshold"), Tango::DevUShort(std::max(0, std::min(100, threshold_))));
			device_.write_attribute(threshold_write);
		}
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
}