#include "VCParams.hpp"

oatpp::Object<VCParamsDTO> vcparams_to_dto(const vc::VideoCaptureDevice::Params& params)
{
	oatpp::Object<VCParamsDTO> result = VCParamsDTO::createShared();

	result->rulerStartX = params.ruler.start.x;
	result->rulerStartY = params.ruler.start.y;
	result->rulerEndX = params.ruler.end.x;
	result->rulerEndY = params.ruler.end.y;
	result->rulerLength = params.ruler.length;
	result->threshold = params.threshold;

	return result;
}

vc::VideoCaptureDevice::Params vcparams_from_dto(const oatpp::Object<VCParamsDTO>& paramsDto)
{
	return { { { paramsDto->rulerStartX.getValue(0), paramsDto->rulerStartY.getValue(0) },
			   { paramsDto->rulerEndX.getValue(0), paramsDto->rulerEndX.getValue(0) },
			   paramsDto->rulerLength.getValue(0) }, 
			  paramsDto->threshold.getValue(0) };
}