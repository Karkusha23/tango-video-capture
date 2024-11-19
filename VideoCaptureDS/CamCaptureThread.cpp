#include "CamCaptureThread.h"

namespace VideoCaptureDS_ns
{
	CamCaptureThread::CamCaptureThread(VideoCaptureDS* dev, int source, int width, int height) :
		omni_thread(), Tango::LogAdapter((TANGO_BASE_CLASS*)dev), device_(dev),
		local_exit_(ATOMIC_VAR_INIT(false)), is_failed_(ATOMIC_VAR_INIT(false)),
		cam_(nullptr), image_no_image(nullptr), jpeg_params_({ cv::IMWRITE_JPEG_QUALITY, 50 })
	{

		connect(source, width, height);
		start_undetached();
	}

	CamCaptureThread::~CamCaptureThread()
	{
		DEBUG_STREAM << "CamCaptureThread::~CamCaptureThread() entering" << std::endl;

		//delete contours_;
		//delete hierarchy_;

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

			cv::cvtColor(*query.image, image_gray, cv::COLOR_BGR2GRAY);

			get_contours_(image_gray, query.contours, query.ruler, query.threshold);
			
			jpeg_params_[1] = query.jpegQuality;

			switch (query.mode)
			{
			case vc::CameraMode::RGB:
				cv::cvtColor(*query.image, image_converted, cv::COLOR_BGR2RGB);
				cv::imencode(".jpg", image_converted, *query.jpeg, jpeg_params_);
				*query.image = std::move(image_converted);
				break;
			case vc::CameraMode::BGR:
				cv::imencode(".jpg", *query.image, *query.jpeg, jpeg_params_);
				break;
			case vc::CameraMode::Grayscale:
				cv::imencode(".jpg", image_gray, *query.jpeg, jpeg_params_);
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

	void CamCaptureThread::capture(cv::Mat* image, std::vector<unsigned char>* jpeg, std::vector<vc::ContourInfo>* contours, const vc::Ruler* ruler,
								   vc::CameraMode mode, double jpegQuality, int threshold, std::atomic_bool* status)
	{
		omni_mutex_lock lock(queue_mutex_);

		DEBUG_STREAM << "CamCaptureThread: Adding capture query to queue" << std::endl;

		queue_.push({ image, jpeg, contours, ruler, mode, jpegQuality, threshold, status });
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

	void CamCaptureThread::get_contours_(const cv::Mat& image_gray, std::vector<vc::ContourInfo>* contourInfo, const vc::Ruler* ruler, int threshold)
	{
		DEBUG_STREAM << "CamCaptureThread: Getting contours" << std::endl;

		cv::Mat image_blur;
		cv::Mat image_canny;
		cv::Mat image_dilation;

		cv::GaussianBlur(image_gray, image_blur, cv::Size(5, 5), 3, 0);
		cv::Canny(image_blur, image_canny, threshold, threshold * 3);
		cv::dilate(image_canny, image_dilation, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

		cv::findContours(image_dilation, contours_, hierarchy_, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for (const std::vector<cv::Point>& contour : contours_)
		{
			int area_rel = cv::contourArea(contour);

			if (area_rel < 1000)
			{
				continue;
			}

			float diameter_rel;
			cv::Point2f center;
			cv::minEnclosingCircle(contour, center, diameter_rel);

			vc::ContourInfo con_info;

			con_info.bound_rect = cv::boundingRect(contour);
			con_info.center_mass = get_center_of_mass_(contour);

			con_info.area_rel = area_rel;
			con_info.diameter_rel = diameter_rel;

			if (is_ruler_valid_(ruler))
			{
				double multiplier = ruler->length / std::sqrt((ruler->start.x - ruler->end.x) * (ruler->start.x - ruler->end.x) + (ruler->start.y - ruler->end.y) * (ruler->start.y - ruler->end.y));

				con_info.area_abs = area_rel * multiplier * multiplier;
				con_info.diameter_abs = (double)diameter_rel * multiplier;
			}
			else
			{
				con_info.area_abs = con_info.diameter_abs = 0.0;
			}

			contourInfo->push_back(con_info);
		}

		DEBUG_STREAM << "CamCaptureThread: Contours found: " << contours_.size() << "; valid ones: " << contourInfo->size() << std::endl;
	}

	cv::Point CamCaptureThread::get_center_of_mass_(const std::vector<cv::Point>& contour)
	{
		size_t x = 0;
		size_t y = 0;

		for (const cv::Point& point : contour)
		{
			x += (size_t)point.x;
			y += (size_t)point.y;
		}

		x /= contour.size();
		y /= contour.size();

		return cv::Point((int)x, (int)y);
	}

	bool CamCaptureThread::is_ruler_valid_(const vc::Ruler* ruler)
	{
		return ruler != nullptr && ruler->start != ruler->end && ruler->length > 0.0;
	}
}