#ifndef MyInters_hpp_included
#define MyInters_hpp_included

#include <oatpp/core/data/mapping/type/Primitive.hpp>
#include <oatpp/core/Types.hpp>

#include "MyDTOs.hpp"
#include "../vcclient/VideoCaptureClient.h"

namespace __class
{
	class VCParamsInter;
	class ContourInfoInter;
}

typedef oatpp::data::mapping::type::Primitive<vc::VideoCaptureDevice::Params, __class::VCParamsInter> VCParamsDTOInter;
typedef oatpp::data::mapping::type::Primitive<vc::ContourInfo, __class::ContourInfoInter> ContourInfoDTOInter;

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

	class ContourInfoInter
	{
		class Inter : public oatpp::Type::Interpretation<ContourInfoDTOInter, oatpp::Object<dto::ContourInfo>>
		{
		public:
			oatpp::Object<dto::ContourInfo> interpret(const ContourInfoDTOInter& value) const override;
			ContourInfoDTOInter reproduce(const oatpp::Object<dto::ContourInfo>& obj) const override;
		};

		static oatpp::Type::Info getTypeInfo();

	public:

		static const oatpp::ClassId CLASS_ID;
		static oatpp::Type* getType();
	};
}

#endif