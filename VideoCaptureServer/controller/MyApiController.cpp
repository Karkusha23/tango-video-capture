#include "MyApiController.hpp"

std::shared_ptr<MyApiController::OutgoingResponse> MyApiController::getStaticFileResponse(const oatpp::String& filename, const oatpp::String& rangeHeader, bool ignore_cache) const
{
	oatpp::String file = staticFileManager->getFile(filename, ignore_cache);

	OATPP_ASSERT_HTTP(file.get() != nullptr, Status::CODE_400, "File not found");

	std::shared_ptr<OutgoingResponse> response;

	if (!rangeHeader)
	{
		response = getFullFileResponse(file);
	}
	else
	{
		response = getRangeResponse(rangeHeader, file);
	}

	response->putHeader("Accept-Ranges", "bytes");
	response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);

	oatpp::String mimeType = getMimeType(filename);
	if (mimeType)
	{
		response->putHeader(Header::CONTENT_TYPE, mimeType);
	}
	else
	{
		OATPP_LOGD("Server", "Unknown Mime Type");
	}

	return response;
}

std::shared_ptr<MyApiController::OutgoingResponse> MyApiController::getFullFileResponse(const oatpp::String& file) const
{
	return createResponse(Status::CODE_200, file);
}

std::shared_ptr<MyApiController::OutgoingResponse> MyApiController::getRangeResponse(const oatpp::String& rangeStr, const oatpp::String& file) const
{
	auto range = oatpp::web::protocol::http::Range::parse(rangeStr.getPtr());

	if (range.end == 0)
	{
		range.end = file->size() - 1;
	}

	OATPP_ASSERT_HTTP(range.isValid() && range.start < file->size() && range.end > range.start && range.end < file->size(), Status::CODE_416, "Range is invalid");

	oatpp::String chunk = oatpp::String(&file->data()[range.start], (v_int32)(range.end - range.start + 1));

	auto response = createResponse(Status::CODE_206, chunk);

	oatpp::web::protocol::http::ContentRange contentRange(oatpp::web::protocol::http::ContentRange::UNIT_BYTES, range.start, range.end, file->size(), true);

	OATPP_LOGD("Server", "range=%s", contentRange.toString()->c_str());

	response->putHeader(Header::CONTENT_RANGE, contentRange.toString());

	return response;
}