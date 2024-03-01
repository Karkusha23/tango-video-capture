#include <iostream>
#include <exception>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "VideoCaptureClient.h"

int main(int argc, char* argv[])
{
	unsigned char* image_byte = nullptr;

	try
	{
		Tango::DeviceProxy device(argc < 2 ? "CVCam/test/0" : argv[1]);

		vcc::print_device_info(std::cout, &device);

		vcc::CameraMode cam_mode = vcc::get_device_camera_mode(&device);

		if (cam_mode == vcc::CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		Tango::DeviceAttribute devAttr;
		Tango::EncodedAttribute enAttr;
		int width, height;

		cv::Mat image_mat;
		cv::Mat image_converted;

		do
		{
			delete[] image_byte;
			image_byte = nullptr;

			devAttr = device.read_attribute("Jpeg");

			switch (cam_mode)
			{
			case vcc::CameraMode::RGB:
				enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_RGBA2BGR);
				break; 
			case vcc::CameraMode::BGR:
				enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_BGRA2BGR);
				break;
			case vcc::CameraMode::Grayscale:
				enAttr.decode_gray8(&devAttr, &width, &height, &image_byte);
				image_mat = cv::Mat(height, width, CV_8UC1, image_byte);
				cv::cvtColor(image_mat, image_converted, cv::COLOR_GRAY2BGR);
				break;
			default:
				break;
			}

			cv::imshow("Image", image_converted);
		} while (cv::waitKey(50) != 0x1B);
	}
	catch (Tango::DevFailed& e)
	{
		delete[] image_byte;
		Tango::Except::print_exception(e);
		return 1;
	}
	catch (std::exception& e)
	{
		delete[] image_byte;
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		delete[] image_byte;
		std::cout << "Unknown exception" << std::endl;
		return 3;
	}

	delete[] image_byte;

	return 0;
}