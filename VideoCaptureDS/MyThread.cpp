#include "MyThread.h"

namespace VideoCaptureDS_ns
{
	MyThread::MyThread(VideoCaptureDS* dev, int source, int width, int height, const std::string& mode, int quality) : omni_thread(), Tango::LogAdapter((TANGO_BASE_CLASS*)dev), device_(dev),
																													   local_exit_(false), to_capture_(false), is_executing_(false), is_failed_(false),
																													   cam_(nullptr), image_no_image(nullptr),
																													   status_(nullptr), image(nullptr), jpeg(nullptr)
	{
		connect(source, width, height, mode, quality);
		start_undetached();
	}

	MyThread::~MyThread()
	{
		delete cam_;
		delete image_no_image;
	}

	void* MyThread::run_undetached(void *ptr)
	{
		DEBUG_STREAM << "My thread stared!" << endl;

		while (!local_exit_)
		{
			if (to_capture_)
			{
				to_capture_ = false;
				
				is_executing_ = true;

				if (is_failed_)
				{
					*image = image_no_image->clone();
				}
				else
				{
					cam_->read(*image);

					if (image->empty())
					{
						*image = image_no_image->clone();
					}
				}

				cv::Mat image_converted;
				cv::Mat image_to_jpeg;

				switch (mode_)
				{
				case CameraMode::RGB:
					cv::cvtColor(*image, image_converted, cv::COLOR_BGR2RGB);
					cv::cvtColor(*image, image_to_jpeg, cv::COLOR_BGR2RGBA);
					jpeg->encode_jpeg_rgb32(image_to_jpeg.data, width_, height_, quality_);
					*image = image_converted.clone();
					break;
				case CameraMode::BGR:
					cv::cvtColor(*image, image_to_jpeg, cv::COLOR_BGR2BGRA);
					jpeg->encode_jpeg_rgb32(image_to_jpeg.data, width_, height_, quality_);
					break;
				case CameraMode::Grayscale:
					cv::cvtColor(*image, image_converted, cv::COLOR_BGR2GRAY);
					jpeg->encode_jpeg_gray8(image_converted.data, width_, height_, quality_);
					*image = image_converted.clone();
					break;
				default:
					break;
				}

				is_executing_ = false;

				*status_ = 1;
			}

			// Sleep(100); // delay if needed
		}

		DEBUG_STREAM << "My thread stopped!" << endl;
		return 0;
	}

	void MyThread::stop()
	{
		local_exit_ = true;
	}

	void MyThread::execute_capture(cv::Mat* image, Tango::EncodedAttribute* jpeg, int* status)
	{
		while (is_executing_)
		{

		}

		to_capture_ = true;
		status_ = status;
		this->image = image;
		this->jpeg = jpeg;
	}

	bool MyThread::is_executing() const
	{
		return is_executing_;
	}

	bool MyThread::is_failed() const
	{
		return is_failed_;
	}

	void MyThread::connect(int source, int width, int height, const std::string& mode, int quality)
	{
		while (is_executing_)
		{
			
		}

		delete cam_;
		delete image_no_image;

		cam_ = new cv::VideoCapture(source);

		width_ = width < 3840 ? width : 3840;
		height_ = height < 720 ? height : 720;
		quality_ = std::max(0, std::min(100, quality));

		if (mode == "RGB" || mode == "rgb")
		{
			mode_ = CameraMode::RGB;
		}
		else if (mode == "BGR" || mode == "bgr")
		{
			mode_ = CameraMode::BGR;
		}
		else if (mode == "Grayscale" || mode == "grayscale")
		{
			mode_ = CameraMode::Grayscale;
		}
		else
		{
			mode_ = CameraMode::None;
			is_failed_ = true;
		}

		if (!cam_->isOpened())
		{
			is_failed_ = true;
		}
		else
		{
			cam_->set(cv::CAP_PROP_FRAME_HEIGHT, height);
			cam_->set(cv::CAP_PROP_FRAME_WIDTH, width);

			if (cam_->get(cv::CAP_PROP_FRAME_HEIGHT) != height || cam_->get(cv::CAP_PROP_FRAME_WIDTH) != width)
			{
				is_failed_ = true;
			}
		}
	}
}