#include "MyDTOs.hpp"

namespace dto
{
	oatpp::Object<dto::Point> todto(const cv::Point& val)
	{
		oatpp::Object<dto::Point> result = dto::Point::createShared();
		result->x = val.x;
		result->y = val.y;
		return result;
	}

	oatpp::Object<dto::Rect> todto(const cv::Rect& val)
	{
		oatpp::Object<dto::Rect> result = dto::Rect::createShared();
		result->tl = todto(val.tl());
		result->br = todto(val.br());
		return result;
	}

	oatpp::Object<dto::Ruler> todto(const vc::Ruler& val)
	{
		oatpp::Object<dto::Ruler> result = dto::Ruler::createShared();
		result->start = todto(val.start);
		result->end = todto(val.end);
		result->length = val.length;
		return result;
	}

	cv::Point fromdto(const oatpp::Object<dto::Point>& obj)
	{
		return cv::Point(obj->x.getValue(0), obj->y.getValue(0));
	}

	cv::Rect fromdto(const oatpp::Object<dto::Rect>& obj)
	{
		return cv::Rect(fromdto(obj->tl), fromdto(obj->br));
	}

	vc::Ruler fromdto(const oatpp::Object<dto::Ruler>& obj)
	{
		return { fromdto(obj->start), fromdto(obj->end), obj->length.getValue(0) };
	}
}