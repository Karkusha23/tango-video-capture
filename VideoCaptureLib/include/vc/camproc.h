#ifndef _CamProcIncluded
#define _CamProcIncluded

#include <opencv2/core.hpp>

namespace vc
{
	enum class CameraMode : unsigned char { None, RGB, BGR, Grayscale };

	struct Ruler
	{
		cv::Point start;
		cv::Point end;
		double length;
	};

	struct ContourInfo
	{
		cv::Rect bound_rect;
		cv::Point center_mass;

		int area_rel;
		double area_abs;

		double diameter_rel;
		double diameter_abs;
	};
}

#endif