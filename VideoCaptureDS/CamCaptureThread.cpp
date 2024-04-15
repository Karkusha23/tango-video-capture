#include "CamCaptureThread.h"

namespace VideoCaptureDS_ns
{
	CamCaptureThread::CamCaptureThread(VideoCaptureDS* dev, int source, int width, int height) :
		omni_thread(), Tango::LogAdapter((TANGO_BASE_CLASS*)dev), device_(dev),
		local_exit_(ATOMIC_VAR_INIT(false)), is_failed_(ATOMIC_VAR_INIT(false)),
		cam_(nullptr), image_no_image(nullptr)
	{
		connect(source, width, height);
		start_undetached();
	}

	CamCaptureThread::~CamCaptureThread()
	{
		delete cam_;
		delete image_no_image;
	}

	void* CamCaptureThread::run_undetached(void *ptr)
	{
		DEBUG_STREAM << "My thread stared!" << endl;

		while (!local_exit_)
		{
			CaptureQuery query;

			{
				omni_mutex_lock lock(queue_mutex_);

				if (queue_.empty())
				{
					continue;
				}

				query = queue_.front();
				queue_.pop();
			}

			{
				omni_mutex_lock lock(cam_mutex_);

				if (is_failed_)
				{
					*query.image = image_no_image->clone();
				}
				else
				{
					cam_->read(*query.image);

					if (query.image->empty())
					{
						*query.image = image_no_image->clone();
					}
				}
			}

			cv::Mat image_converted;
			cv::Mat image_to_jpeg;

			switch (query.mode)
			{
			case CameraMode::RGB:
				cv::cvtColor(*query.image, image_converted, cv::COLOR_BGR2RGB);
				cv::cvtColor(*query.image, image_to_jpeg, cv::COLOR_BGR2RGBA);
				query.jpeg->encode_jpeg_rgb32(image_to_jpeg.data, width_, height_, query.jpegQuality);
				*query.image = std::move(image_converted);
				break;
			case CameraMode::BGR:
				cv::cvtColor(*query.image, image_to_jpeg, cv::COLOR_BGR2BGRA);
				query.jpeg->encode_jpeg_rgb32(image_to_jpeg.data, width_, height_, query.jpegQuality);
				break;
			case CameraMode::Grayscale:
				cv::cvtColor(*query.image, image_converted, cv::COLOR_BGR2GRAY);
				query.jpeg->encode_jpeg_gray8(image_converted.data, width_, height_, query.jpegQuality);
				*query.image = std::move(image_converted);
				break;
			default:
				break;
			}

			*query.status = true;

			// Sleep(100); // delay if needed
		}

		DEBUG_STREAM << "My thread stopped!" << endl;
		return 0;
	}

	void CamCaptureThread::stop()
	{
		local_exit_ = true;
	}

	void CamCaptureThread::execute_capture(cv::Mat* image, Tango::EncodedAttribute* jpeg, CameraMode mode, double jpegQuality, bool* status)
	{
		omni_mutex_lock lock(queue_mutex_);
		queue_.push({ image, jpeg, mode, jpegQuality, status });
	}

	bool CamCaptureThread::is_failed() const
	{
		return is_failed_;
	}

	void CamCaptureThread::connect(int source, int width, int height)
	{
		omni_mutex_lock lock(cam_mutex_);

		delete cam_;
		delete image_no_image;

		cam_ = new cv::VideoCapture(source);

		width_ = width < 3840 ? width : 3840;
		height_ = height < 720 ? height : 720;

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

		image_no_image = new cv::Mat(height, width, CV_8UC3, cv::Scalar(255, 0, 0));
		cv::putText(*image_no_image, "No image", cv::Point(width / 2 - 50, height / 2), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	}
}