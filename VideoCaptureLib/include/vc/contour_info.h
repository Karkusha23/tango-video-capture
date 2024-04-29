#ifndef ContourInfoIncluded
#define ContourInfoIncluded

#include <opencv2/core.hpp>

namespace vc
{
	struct ContourInfo
	{
		cv::Rect boundRect;
		int area;
		double perimeter;
	};
}

#endif