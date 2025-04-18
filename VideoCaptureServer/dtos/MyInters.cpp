#include "MyInters.hpp"

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

	const oatpp::ClassId ContourInfoInter::CLASS_ID("my-types::ContourInfoDTOInter");

	oatpp::Object<dto::ContourInfo> ContourInfoInter::Inter::interpret(const ContourInfoDTOInter& value) const
	{
		return dto::todto(vc::ContourInfo(value));
	}

	ContourInfoDTOInter ContourInfoInter::Inter::reproduce(const oatpp::Object<dto::ContourInfo>& obj) const
	{
		return dto::fromdto(obj);
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