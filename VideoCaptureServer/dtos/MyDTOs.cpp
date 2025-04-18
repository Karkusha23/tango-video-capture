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

	oatpp::Object<dto::VCParams> todto(const vc::VideoCaptureDevice::Params& val)
	{
		oatpp::Object<dto::VCParams> result = dto::VCParams::createShared();
		result->ruler = todto(val.ruler);
		result->threshold = val.threshold;
		result->minContourArea = val.minContourArea;
		return result;
	}

	oatpp::Object<dto::ContourInfo> todto(const vc::ContourInfo& val)
	{
		oatpp::Object<dto::ContourInfo> result = dto::ContourInfo::createShared();
		result->boundingRect = todto(val.bound_rect);
		result->centerMass = todto(val.center_mass);
		result->areaRel = val.area_rel;
		result->areaAbs = val.area_abs;
		result->diameterRel = val.diameter_rel;
		result->diameterAbs = val.diameter_abs;
		return result;
	}

	oatpp::Object<dto::ContourStamp> todto(const vc::ContourStamp& val)
	{
		oatpp::Object<dto::ContourStamp> result = dto::ContourStamp::createShared();
		result->pts = val.pts;
		result->infos = {};
		for (const vc::ContourInfo& it : val.infos)
		{
			result->infos->push_back(todto(it));
		}
		return result;
	}

	oatpp::Object<dto::ContourList> todto(const std::vector<vc::ContourStamp>& val)
	{
		oatpp::Object<dto::ContourList> result = dto::ContourList::createShared();
		result->list = {};
		for (const vc::ContourStamp& it : val)
		{
			result->list->push_back(todto(it));
		}
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

	vc::VideoCaptureDevice::Params fromdto(const oatpp::Object<dto::VCParams>& obj)
	{
		return { fromdto(obj->ruler), obj->threshold.getValue(0), obj->minContourArea.getValue(0) };
	}

	vc::ContourInfo fromdto(const oatpp::Object<dto::ContourInfo>& obj)
	{
		return { fromdto(obj->boundingRect), fromdto(obj->centerMass), obj->areaRel.getValue(0), obj->areaAbs.getValue(0), obj->diameterRel.getValue(0), obj->diameterAbs.getValue(0) };
	}

	vc::ContourStamp fromdto(const oatpp::Object<dto::ContourStamp>& obj)
	{
		std::vector<vc::ContourInfo> infos;
		for (const oatpp::Object<dto::ContourInfo>& it : *(obj->infos))
		{
			infos.push_back(fromdto(it));
		}
		return { obj->pts.getValue(0), infos };
	}

	std::vector<vc::ContourStamp> fromdto(const oatpp::Object<dto::ContourList>& obj)
	{
		std::vector<vc::ContourStamp> result;
		for (const oatpp::Object<dto::ContourStamp>& it : *(obj->list))
		{
			result.push_back(fromdto(it));
		}
		return result;
	}
}