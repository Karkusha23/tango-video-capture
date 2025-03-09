#ifndef VCParamsInter_hpp_included
#define VCParamsInter_hpp_included

#include <oatpp/core/data/mapping/type/Primitive.hpp>
#include <oatpp/core/Types.hpp>

#include "MyDTOs.hpp"
#include "../vcclient/VideoCaptureClient.h"

namespace __class
{
	class VCParamsInter;
}

typedef oatpp::data::mapping::type::Primitive<vc::VideoCaptureDevice::Params, __class::VCParamsInter> VCParamsDTOInter;

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
}

#endif