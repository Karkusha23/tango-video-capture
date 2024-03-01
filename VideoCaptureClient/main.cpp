#include <iostream>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main(int argc, char* argv[])
{
	unsigned char* image_byte = nullptr;

	try
	{
		Tango::DeviceProxy device(argc < 2 ? "CVCam/test/0" : argv[1]);

		std::cout << "Ping time: " << device.ping() << std::endl;
		std::cout << "Device name:" << device.name() << std::endl;

		Tango::DbData properties;
		properties.push_back(Tango::DbDatum("Mode"));
		device.get_property(properties);

		std::cout << properties.size() << std::endl;

		Tango::DbData mode_db;
		std::string mode;
		device.get_property(std::string("Mode"), mode_db);
		properties[0] >> mode;
		std::cout << "Camera mode: " << mode << std::endl;

		Tango::DeviceAttribute devAttr;
		Tango::EncodedAttribute enAttr;
		int width, height;

		do
		{
			delete[] image_byte;
			image_byte = nullptr;

			devAttr = device.read_attribute("Jpeg");
			enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);

			cv::Mat image_mat(height, width, CV_8UC4, image_byte);
			cv::Mat image_converted;

			cv::cvtColor(image_mat, image_converted, cv::COLOR_RGBA2BGR);

			cv::imshow("Image", image_converted);
		} while (cv::waitKey(50) != 0x1B);
	}
	catch (Tango::DevFailed& e)
	{
		delete[] image_byte;
		Tango::Except::print_exception(e);
		system("pause");
		return 1;
	}
	catch (...)
	{
		delete[] image_byte;
		std::cout << "Unknown exception" << std::endl;
		system("pause");
		return 2;
	}

	delete[] image_byte;

	return 0;
}