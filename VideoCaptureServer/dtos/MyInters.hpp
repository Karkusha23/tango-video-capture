#ifndef MyInters_hpp_included
#define MyInters_hpp_included

#include <oatpp/core/data/mapping/type/Primitive.hpp>
#include <oatpp/core/Types.hpp>

#include "MyDTOs.hpp"
#include "../vcclient/VideoCaptureClient.h"

namespace dto
{
	oatpp::Object<dto::VCParams> todto(const vc::VideoCaptureDevice::Params& val);
	vc::VideoCaptureDevice::Params fromdto(const oatpp::Object<dto::VCParams>& obj);
}

namespace __class
{
	class VCParamsInter;
	class ContourListInter;
}

typedef oatpp::data::mapping::type::Primitive<vc::VideoCaptureDevice::Params, __class::VCParamsInter> VCParamsDTOInter;
typedef oatpp::data::mapping::type::Primitive<std::vector<vc::ContourStamp>, __class::ContourListInter> ContourListDTOInter;

namespace __class
{
	class VCParamsInter
	{
		class Inter : public oatpp::Type::Interpretation<VCParamsDTOInter, oatpp::Object<dto::VCParams>>
		{
		public:
			oatpp::Object<dto::VCParams> interpret(const VCParamsDTOInter& value) const override;
			VCParamsDTOInter reproduce(const oatpp::Object<dto::VCParams>& obj) const override;
		};

		static oatpp::Type::Info getTypeInfo();

	public:

		static const oatpp::ClassId CLASS_ID;
		static oatpp::Type* getType();
	};

	class ContourListInter
	{
		class Inter : public oatpp::Type::Interpretation<ContourListDTOInter, oatpp::Object<dto::ContourList>>
		{
		public:
			oatpp::Object<dto::ContourList> interpret(const ContourListDTOInter& value) const override;
			ContourListDTOInter reproduce(const oatpp::Object<dto::ContourList>& obj) const override;
		};

		static oatpp::Type::Info getTypeInfo();

	public:

		static const oatpp::ClassId CLASS_ID;
		static oatpp::Type* getType();
	};
}

#endif