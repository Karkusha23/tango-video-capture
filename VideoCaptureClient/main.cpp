#include <iostream>
#include <exception>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vc/camproc.h>

#include "VideoCaptureClient.h"

int main(int argc, char* argv[])
{
	try
	{
		vc::VideoCaptureDevice dev(argc < 2 ? "CVCam/test/0" : argv[1]);
		int update_time = argc < 3 ? 300 : std::stoi(argv[2]);

		dev.print_device_info(std::cout);

		vc::JpegCallBack callBack(&dev);
		int event_id = dev.device().subscribe_event(std::string("Jpeg"), Tango::CHANGE_EVENT, &callBack, std::vector<std::string>());

		do
		{
			dev.update();
		} while (cv::waitKey(update_time) != 0x1B);

		dev.device().unsubscribe_event(event_id);
	}
	catch (Tango::DevFailed& e)
	{
		Tango::Except::print_exception(e);
		return 1;
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		std::cout << "Unknown exception" << std::endl;
		return 3;
	}

	return 0;
}