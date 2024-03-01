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
		std::cout << "Device name: " << device.name() << std::endl;

		std::vector<std::string> device_properties;
		device.get_property_list("*", device_properties);
		std::cout << "Device Properties:" << std::endl;
		for (std::string& device_property : device_properties)
		{
			std::cout << '\t' << device_property << ": ";
			Tango::DbData property_data;
			device.get_property(device_property, property_data);
			for (Tango::DbDatum& data : property_data)
			{
				std::string outstring;
				data >> outstring;
				std::cout << outstring << ' ';
			}
			std::cout << std::endl;
		}

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