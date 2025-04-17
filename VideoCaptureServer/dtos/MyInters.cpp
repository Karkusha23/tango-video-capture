#include "MyInters.hpp"

namespace __class
{
	const oatpp::ClassId VCParamsInter::CLASS_ID("my-types::VCParamsDTOInter");

	oatpp::Object<dto::VCParams> VCParamsInter::Inter::interpret(const VCParamsDTOInter& value) const
	{
		oatpp::Object<dto::VCParams> result = dto::VCParams::createShared();

		result->ruler = dto::Ruler::createShared();

		result->ruler->start = dto::Point::createShared();
		result->ruler->end = dto::Point::createShared();

		result->ruler->start->x = value->ruler.start.x;
		result->ruler->start->y = value->ruler.start.y;
		result->ruler->end->x = value->ruler.end.x;
		result->ruler->end->y = value->ruler.end.y;
		result->ruler->length = value->ruler.length;
		result->threshold = value->threshold;
		result->minContourArea = value->minContourArea;

		return result;
	}

	VCParamsDTOInter VCParamsInter::Inter::reproduce(const oatpp::Object<dto::VCParams>& obj) const
	{
		cv::Point start(obj->ruler->start->x.getValue(0), obj->ruler->start->y.getValue(0));
		cv::Point end(obj->ruler->end->x.getValue(0), obj->ruler->end->y.getValue(0));

		vc::Ruler ruler = { start, end, obj->ruler->length.getValue(0) };

		return vc::VideoCaptureDevice::Params({ ruler, obj->threshold.getValue(0), obj->minContourArea.getValue(0) });
	}

	oatpp::Type::Info VCParamsInter::getTypeInfo()
	{
		oatpp::Type::Info info;
		info.interpretationMap.insert({ "my-types", new Inter() });
		return info;
	}

	oatpp::Type* VCParamsInter::getType()
	{
		static oatpp::Type type(CLASS_ID, getTypeInfo());
		return &type;
	}

	const oatpp::ClassId ContourInfoInter::CLASS_ID("my-types::ContourInfoDTOInter");

	oatpp::Object<dto::ContourInfo> ContourInfoInter::Inter::interpret(const ContourInfoDTOInter& value) const
	{
		oatpp::Object<dto::ContourInfo> result = dto::ContourInfo::createShared();

		result->boundingRect = dto::Rect::createShared();
		result->boundingRect->tl = dto::Point::createShared();
		result->boundingRect->br = dto::Point::createShared();
		result->centerMass = dto::Point::createShared();

		result->boundingRect->tl->x = value->bound_rect.tl().x;
		result->boundingRect->tl->y = value->bound_rect.tl().y;
		result->boundingRect->br->x = value->bound_rect.br().x;
		result->boundingRect->br->y = value->bound_rect.br().y;

		result->centerMass->x = value->center_mass.x;
		result->centerMass->y = value->center_mass.y;

		result->areaRel = value->area_rel;
		result->areaAbs = value->area_abs;
		result->diameterRel = value->diameter_rel;
		result->diameterAbs = value->diameter_abs;

		return result;
	}

	ContourInfoDTOInter ContourInfoInter::Inter::reproduce(const oatpp::Object<dto::ContourInfo>& obj) const
	{
		cv::Point tl(obj->boundingRect->tl->x.getValue(0), obj->boundingRect->tl->y.getValue(0));
		cv::Point br(obj->boundingRect->br->x.getValue(0), obj->boundingRect->br->y.getValue(0));
		cv::Point centerMass(obj->centerMass->x.getValue(0), obj->centerMass->y.getValue(0));

		cv::Rect boundingRect(tl, br);
		
		return vc::ContourInfo({ boundingRect, centerMass, obj->areaRel.getValue(0), obj->areaAbs.getValue(0), obj->diameterRel.getValue(0), obj->diameterAbs.getValue(0) });
	}

	oatpp::Type::Info ContourInfoInter::getTypeInfo()
	{
		oatpp::Type::Info info;
		info.interpretationMap.insert({ "my-types", new Inter() });
		return info;
	}

	oatpp::Type* ContourInfoInter::getType()
	{
		static oatpp::Type type(CLASS_ID, getTypeInfo());
		return &type;
	}
}