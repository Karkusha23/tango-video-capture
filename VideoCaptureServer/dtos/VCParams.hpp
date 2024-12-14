#ifndef VCParams_hpp_included
#define VCParams_hpp_included

#include <oatpp/core/Types.hpp>
#include <oatpp/core/macro/codegen.hpp>

#include "../vcclient/VideoCaptureClient.h"

#include OATPP_CODEGEN_BEGIN(DTO)

class VCParamsDTO : public oatpp::DTO
{
	DTO_INIT(VCParamsDTO, DTO);

	DTO_FIELD(Int32, threshold);
	DTO_FIELD(Int32, rulerStartX);
	DTO_FIELD(Int32, rulerStartY);
	DTO_FIELD(Int32, rulerEndX);
	DTO_FIELD(Int32, rulerEndY);
	DTO_FIELD(Float64, rulerLength);
};

#include OATPP_CODEGEN_END(DTO)

oatpp::Object<VCParamsDTO> vcparams_to_dto(const vc::VideoCaptureDevice::Params& params);
vc::VideoCaptureDevice::Params vcparams_from_dto(const oatpp::Object<VCParamsDTO>& paramsDto);

#endif