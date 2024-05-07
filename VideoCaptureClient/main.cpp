#include <iostream>
#include <exception>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

#include "VideoCaptureClient.h"

Tango::DeviceProxy* device = nullptr;

vc::CameraMode cam_mode;

unsigned char* image_byte = nullptr;

Tango::DeviceAttribute devAttr;
Tango::EncodedAttribute enAttr;

cv::Mat image_mat;
cv::Mat image_converted;

int threshold = 0;
int threshold_prev = threshold_prev;

void event_function_Jpeg(Tango::EventData* event_data);
void event_function_Frame(Tango::EventData* event_data);

int main(int argc, char* argv[])
{
	try
	{
		device = new Tango::DeviceProxy(argc < 2 ? "CVCam/test/0" : argv[1]);
		int update_time = argc < 3 ? 300 : std::stoi(argv[2]);

		vc::print_device_info(std::cout, device);
		
		cam_mode = vc::get_device_camera_mode(device);

		if (cam_mode == vc::CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		//vcc::JpegCallBack callBack(event_function_Jpeg);
		//int event_id = device->subscribe_event(std::string("Jpeg"), Tango::CHANGE_EVENT, &callBack, std::vector<std::string>());

		//vcc::JpegCallBack callBack(event_function_Frame);
		//int event_id = device->subscribe_event(std::string("Frame"), Tango::CHANGE_EVENT, &callBack, std::vector<std::string>());

		//threshold = threshold_prev = vc::get_device_int_property(device, "Threshold");

		threshold = threshold_prev = device->read_attribute("Threshold").UShortSeq[0];

		cv::namedWindow("Threshold", (640, 200));
		cv::createTrackbar("Threshold", "Threshold", &threshold, 100);

		do
		{
			event_function_Jpeg(nullptr);
		} while (cv::waitKey(update_time) != 0x1B);

		//device->unsubscribe_event(event_id);
	}
	catch (Tango::DevFailed& e)
	{
		delete[] device;
		delete[] image_byte;
		Tango::Except::print_exception(e);
		system("pause");
		return 1;
	}
	catch (std::exception& e)
	{
		delete[] device;
		delete[] image_byte;
		std::cout << e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		delete[] device;
		delete[] image_byte;
		std::cout << "Unknown exception" << std::endl;
		return 3;
	}

	delete[] image_byte;

	return 0;
}

void event_function_Jpeg(Tango::EventData* event_data)
{
	int width, height;

	delete[] image_byte;
	image_byte = nullptr;

	devAttr = device->read_attribute("Jpeg");

	Tango::DeviceAttribute contourAttr = device->read_attribute("ContourInfo");

	vc::ContourInfo* contours = reinterpret_cast<vc::ContourInfo*>(contourAttr.EncodedSeq[0].encoded_data.NP_data());
	int contour_count = contourAttr.EncodedSeq[0].encoded_data.length() / sizeof(vc::ContourInfo);

	switch (cam_mode)
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

	for (int i = 0; i < contour_count; ++i)
	{
		std::string text = "Area:" + std::to_string(contours[i].area) /* + "; Perimeter: " + std::to_string(contours[i].perimeter)*/;

		cv::rectangle(image_converted, contours[i].boundRect.tl(), contours[i].boundRect.br(), cv::Scalar(0, 255, 0), 5);
		cv::putText(image_converted, text, { contours[i].boundRect.x, contours[i].boundRect.y - 5 }, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0));
	}

	cv::imshow("Image", image_converted);

	if (threshold != threshold_prev)
	{
		threshold_prev = threshold;
		Tango::DeviceAttribute threshold_write(std::string("Threshold"), Tango::DevUShort(std::max(0, std::min(100, threshold))));
		device->write_attribute(threshold_write);
	}
}

void event_function_Frame(Tango::EventData* event_data)
{
	std::cout << event_data->attr_name << std::endl;
}
