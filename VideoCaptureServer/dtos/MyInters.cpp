#include "MyInters.hpp"

namespace dto
{
	oatpp::Object<dto::VCParams> todto(const vc::VideoCaptureDevice::Params& val)
	{
		oatpp::Object<dto::VCParams> result = dto::VCParams::createShared();
		result->ruler = todto(val.ruler);
		result->threshold = val.threshold;
		result->minContourArea = val.minContourArea;
		return result;
	}

	vc::VideoCaptureDevice::Params fromdto(const oatpp::Object<dto::VCParams>& obj)
	{
		return { fromdto(obj->ruler), obj->threshold.getValue(0), obj->minContourArea.getValue(0) };
	}
}

namespace __class
{
	const oatpp::ClassId VCParamsInter::CLASS_ID("my-types::VCParamsDTOInter");

	oatpp::Object<dto::VCParams> VCParamsInter::Inter::interpret(const VCParamsDTOInter& value) const
	{
		return dto::todto(vc::VideoCaptureDevice::Params(value));
	}

	VCParamsDTOInter VCParamsInter::Inter::reproduce(const oatpp::Object<dto::VCParams>& obj) const
	{
		return dto::fromdto(obj);
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

	const oatpp::ClassId ContourListInter::CLASS_ID("my-types::ContourInfoDTOInter");

	oatpp::Object<dto::ContourList> ContourListInter::Inter::interpret(const ContourListDTOInter& value) const
	{
		return dto::todto(value);
	}

	ContourListDTOInter ContourListInter::Inter::reproduce(const oatpp::Object<dto::ContourList>& obj) const
	{
		return dto::fromdto(obj);
	}

	oatpp::Type::Info ContourListInter::getTypeInfo()
	{
		oatpp::Type::Info info;
		info.interpretationMap.insert({ "my-types", new Inter() });
		return info;
	}

	oatpp::Type* ContourListInter::getType()
	{
		static oatpp::Type type(CLASS_ID, getTypeInfo());
		return &type;
	}
}