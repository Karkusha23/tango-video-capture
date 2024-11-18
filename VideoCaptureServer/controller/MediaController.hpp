#ifndef MediaController_hpp_included
#define MediaController_hpp_included

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>

#include "../filemanager/StaticFileManager.hpp"
#include "../playlist/Playlist.hpp"

#include <chrono>

class MediaController : public oatpp::web::server::api::ApiController
{
public:

	typedef MediaController __ControllerType;

	MediaController(const std::shared_ptr<ObjectMapper>& objectMapper) : oatpp::web::server::api::ApiController(objectMapper) {}
	virtual ~MediaController() {};

	static std::shared_ptr<MediaController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
	{
		return std::make_shared<MediaController>(objectMapper);
	}
	

private:

	OATPP_COMPONENT(std::shared_ptr<StaticFileManager>, staticFileManager);
	OATPP_COMPONENT(std::shared_ptr<Playlist>, playlist);

	std::shared_ptr<OutgoingResponse> getStaticFileResponse(const oatpp::String& filename, const oatpp::String& rangeHeader, bool ignore_cache = false) const;
	std::shared_ptr<OutgoingResponse> getFullFileResponse(const oatpp::String& file) const;
	std::shared_ptr<OutgoingResponse> getRangeResponse(const oatpp::String& rangeStr, const oatpp::String& file) const;

public:

#include OATPP_CODEGEN_BEGIN(ApiController)

	ENDPOINT_ASYNC("GET", "/", Root)
	{
		ENDPOINT_ASYNC_INIT(Root);

		const char* pageTemplate = controller->staticFileManager->getFile("templates/index.html")->c_str();

		Action act() override
		{
			return _return(controller->createResponse(Status::CODE_200, pageTemplate));
		}
	};

	ENDPOINT_ASYNC("GET", "media_no_cache/*", Dynamic)
	{
		ENDPOINT_ASYNC_INIT(Dynamic);

		Action act() override
		{
			oatpp::String filename = request->getPathTail();
			OATPP_ASSERT_HTTP(filename, Status::CODE_400, "Filename is empty");
			auto range = request->getHeader(Header::RANGE);
			return _return(controller->getStaticFileResponse(filename, range, true));
		}
	};

	ENDPOINT_ASYNC("GET", "media/*", Static)
	{
		ENDPOINT_ASYNC_INIT(Static);

		Action act() override
		{
			oatpp::String filename = request->getPathTail();
			OATPP_ASSERT_HTTP(filename, Status::CODE_400, "Filename is empty");
			auto range = request->getHeader(Header::RANGE);
			return _return(controller->getStaticFileResponse(filename, range));
		}
	};

#include OATPP_CODEGEN_END(ApiController)

};

#endif