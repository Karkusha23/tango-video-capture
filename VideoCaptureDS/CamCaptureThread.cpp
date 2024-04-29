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
		DEBUG_STREAM << "CamCaptureThread::~CamCaptureThread() entering" << std::endl;

		delete cam_;
		delete image_no_image;
	}

	void* CamCaptureThread::run_undetached(void *ptr)
	{
		DEBUG_STREAM << "CamCaptureThread: Thread stared!" << std::endl;

		while (!local_exit_)
		{
			CaptureQuery query;

			{
				omni_mutex_lock lock(queue_mutex_);

				if (queue_.empty())
				{
					continue;
				}

				DEBUG_STREAM << "CamCaptureThread: Getting query from queue. Total queries: " << queue_.size() << std::endl;

				query = queue_.front();
				queue_.pop();
			}

			{
				omni_mutex_lock lock(cam_mutex_);

				DEBUG_STREAM << "CamCaptureThread: Reading image from webcam" << std::endl;

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

			DEBUG_STREAM << "CamCaptureThread: Processing image" << std::endl;

			cv::Mat image_converted;
			cv::Mat image_gray;
			cv::Mat image_to_jpeg;

			cv::cvtColor(*query.image, image_gray, cv::COLOR_BGR2GRAY);

			get_contours_(image_gray, query.contours, query.threshold);

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
				query.jpeg->encode_jpeg_gray8(image_gray.data, width_, height_, query.jpegQuality);
				*query.image = std::move(image_gray);
				break;
			default:
				break;
			}

			*query.status = true;

			DEBUG_STREAM << "CamCaptureThread: End of capture" << std::endl;

			// Sleep(100); // delay if needed
		}

		DEBUG_STREAM << "CamCaptureThread: Thread stopped!" << std::endl;

		return 0;
	}

	void CamCaptureThread::stop()
	{
		local_exit_ = true;

		DEBUG_STREAM << "CamCaptureThread: Thread is stopping" << std::endl;
	}

	void CamCaptureThread::capture(cv::Mat* image, Tango::EncodedAttribute* jpeg, std::vector<vc::ContourInfo>* contours, CameraMode mode, double jpegQuality, int threshold, std::atomic_bool* status)
	{
		omni_mutex_lock lock(queue_mutex_);

		DEBUG_STREAM << "CamCaptureThread: Adding capture query to queue" << std::endl;

		queue_.push({ image, jpeg, contours, mode, jpegQuality, threshold, status });
	}

	bool CamCaptureThread::is_failed() const
	{
		return is_failed_;
	}

	void CamCaptureThread::connect(int source, int width, int height)
	{
		omni_mutex_lock lock(cam_mutex_);

		DEBUG_STREAM << "CamCaptureThread: Connecting to cam" << std::endl;

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

	void CamCaptureThread::get_contours_(const cv::Mat& image_gray, std::vector<vc::ContourInfo>* contourInfo, int threshold)
	{
		DEBUG_STREAM << "CamCaptureThread: Getting contours" << std::endl;

		cv::Mat image_blur;
		cv::Mat image_canny;
		cv::Mat image_dilation;

		cv::GaussianBlur(image_gray, image_blur, cv::Size(5, 5), 3, 0);
		cv::Canny(image_blur, image_canny, threshold, threshold * 2.5);
		cv::dilate(image_canny, image_dilation, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

		cv::findContours(image_dilation, contours_, hierarchy_, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for (const std::vector<cv::Point>& contour : contours_)
		{
			vc::ContourInfo con_info;

			con_info.area = cv::contourArea(contour);

			if (con_info.area < 1000)
			{
				continue;
			}

			con_info.boundRect = cv::boundingRect(contour);
			con_info.perimeter = cv::arcLength(contour, true);

			contourInfo->push_back(con_info);
		}

		DEBUG_STREAM << "CamCaptureThread: Contours found: " << contours_.size() << "; valid ones: " << contourInfo->size() << std::endl;
	}
}