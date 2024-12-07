#ifndef MyApiController_hpp_included
#define MyApiController_hpp_included

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>

#include "../filemanager/StaticFileManager.hpp"
#include "../vccmanager/VCCManager.hpp"

// Api controller for Oat++ server that defines processable urls, corresponding actions and responses

class MyApiController : public oatpp::web::server::api::ApiController
{
public:

	typedef MyApiController __ControllerType;

	MyApiController(const std::shared_ptr<ObjectMapper>& objectMapper) : oatpp::web::server::api::ApiController(objectMapper) {}
	virtual ~MyApiController() {};

	static std::shared_ptr<MyApiController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
	{
		return std::make_shared<MyApiController>(objectMapper);
	}

private:

	OATPP_COMPONENT(std::shared_ptr<StaticFileManager>, staticFileManager);
	OATPP_COMPONENT(std::shared_ptr<VCCManager>, vccManager);

	std::shared_ptr<OutgoingResponse> getStaticFileResponse(const oatpp::String& filename, const oatpp::String& rangeHeader, bool ignore_cache = false) const;
	std::shared_ptr<OutgoingResponse> getFullFileResponse(const oatpp::String& file) const;
	std::shared_ptr<OutgoingResponse> getRangeResponse(const oatpp::String& rangeStr, const oatpp::String& file) const;

public:

#include OATPP_CODEGEN_BEGIN(ApiController)

	// Root page
	ENDPOINT_ASYNC("GET", "/", Root)
	{
		ENDPOINT_ASYNC_INIT(Root);

		const char* pageTemplate = controller->staticFileManager->getFile("templates/index.html")->c_str();

		Action act() override
		{
			return _return(controller->createResponse(Status::CODE_200, pageTemplate));
		}
	};

	// Heartbeat of Video Capture Device
	// Fronted JS application send GET request to this address for server to know that this particular device is needed
	// If no heartbeat sent for certain device in 10 seconds, thread that process this device is shut down
	ENDPOINT_ASYNC("GET", "device/{domain}/{group}/{instance}/heartbeat", Heartbeat)
	{
		ENDPOINT_ASYNC_INIT(Heartbeat);

		Action act() override
		{
			std::string domain = request->getPathVariable("domain");
			std::string group = request->getPathVariable("group");
			std::string instance = request->getPathVariable("instance");

			std::string device_name = domain + "/" + group + "/" + instance;

			std::cout << "Heartbeat " << device_name << std::endl;

			bool res = controller->vccManager->heartBeat(device_name);

			OATPP_ASSERT_HTTP(res, Status::CODE_400, "Not connected to device");

			return _return(controller->createResponse(Status::CODE_200, "OK"));
		}
	};

	// Page of device
	// On this page user can view video stream from device and change its properties
	ENDPOINT_ASYNC("GET", "device/{domain}/{group}/{instance}", Device)
	{
		ENDPOINT_ASYNC_INIT(Device);

		const char* pageTemplate = controller->staticFileManager->getFile("templates/device.html")->c_str();

		Action act() override
		{
			std::string domain = request->getPathVariable("domain");
			std::string group = request->getPathVariable("group");
			std::string instance = request->getPathVariable("instance");

			std::string device_name = domain + "/" + group + "/" + instance;

			bool res = controller->vccManager->connectDevice(device_name);

			OATPP_ASSERT_HTTP(res, Status::CODE_400, "Can not connect to device");

			oatpp::String response = formatText(pageTemplate, device_name.c_str());

			return _return(controller->createResponse(Status::CODE_200, response->c_str()));
		}
	};

	// Get file from server file folder ignoring cache
	// Ignoring cache is necessary when getting .m3u8 playlist file because it is constantly changing
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

	// Response with "1" if certain file exist in server file folder, "0" otherwise
	ENDPOINT_ASYNC("GET", "media_exists/*", MediaExists)
	{
		ENDPOINT_ASYNC_INIT(MediaExists);

		Action act() override
		{
			oatpp::String filename = request->getPathTail();
			OATPP_ASSERT_HTTP(filename, Status::CODE_400, "Filename is empty");
			bool exists = controller->staticFileManager->isFileExists(filename);
			return _return(controller->createResponse(Status::CODE_200, exists ? "1" : "0"));
		}
	};

	// Get file from server file folder
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