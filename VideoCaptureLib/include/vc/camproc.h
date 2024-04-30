#ifndef _CamProcIncluded
#define _CamProcIncluded

#include <opencv2/core.hpp>

namespace vc
{
	enum class CameraMode : unsigned char { None, RGB, BGR, Grayscale };

	struct ContourInfo
	{
		cv::Rect boundRect;
		int area;
		double perimeter;
	};
}

#endif