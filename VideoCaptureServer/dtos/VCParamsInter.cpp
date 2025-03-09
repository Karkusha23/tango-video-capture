#include "VCParamsInter.hpp"

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
}