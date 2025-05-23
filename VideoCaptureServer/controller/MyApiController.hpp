#ifndef MyApiController_hpp_included
#define MyApiController_hpp_included

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>

#include "../filemanager/StaticFileManager.hpp"
#include "../vccmanager/VCCManager.hpp"
#include "../dtos/MyInters.hpp"

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

	OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper);
	OATPP_COMPONENT(std::shared_ptr<StaticFileManager>, staticFileManager);
	OATPP_COMPONENT(std::shared_ptr<VCCManager>, vccManager);

	std::shared_ptr<OutgoingResponse> getStaticFileResponse(const oatpp::String& filename, const oatpp::String& rangeHeader, bool ignore_cache = false) const;
	std::shared_ptr<OutgoingResponse> getFullFileResponse(const oatpp::String& file) const;
	std::shared_ptr<OutgoingResponse> getRangeResponse(const oatpp::String& rangeStr, const oatpp::String& file) const;

	std::string getDeviceName(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) const;

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

	// Page of device
	// On this page user can view video stream from device and change its properties
	ENDPOINT_ASYNC("GET", "device/{domain}/{group}/{instance}", Device)
	{
		ENDPOINT_ASYNC_INIT(Device);

		const char* pageTemplate = controller->staticFileManager->getFile("templates/device.html")->c_str();

		Action act() override
		{
			std::string device_name = controller->getDeviceName(request);

			auto info = controller->vccManager->connectDeviceEncoder(device_name);
			std::string encoderName = info.first;
			std::shared_ptr<vc::VideoCaptureDevice> vc_device = info.second;

			OATPP_ASSERT_HTTP(vc_device, Status::CODE_400, "Can not connect to device");

			std::string outwidth = std::to_string(vc_device->out_width(vc::VideoCaptureDevice::UIDisplayType::SidePanel));
			std::string outheight = std::to_string(vc_device->out_height(vc::VideoCaptureDevice::UIDisplayType::SidePanel));
			std::string camwidth = std::to_string(vc_device->cam_width());
			std::string camheight = std::to_string(vc_device->cam_height());

			oatpp::String response = formatText(pageTemplate, outwidth.c_str(), outheight.c_str(), device_name.c_str(), encoderName.c_str(), camwidth.c_str(), camheight.c_str());

			return _return(controller->createResponse(Status::CODE_200, response->c_str()));
		}
	};

	ENDPOINT_ASYNC("GET", "device/{domain}/{group}/{instance}/params", GetDeviceParams)
	{
		ENDPOINT_ASYNC_INIT(GetDeviceParams);

		Action act() override
		{
			std::string device_name = controller->getDeviceName(request);

			std::shared_ptr<vc::VideoCaptureDevice> vc_device = controller->vccManager->device(device_name);

			OATPP_ASSERT_HTTP(vc_device, Status::CODE_400, "Not connected to device");

			return _return(controller->createResponse(Status::CODE_200, controller->apiObjectMapper->writeToString(VCParamsDTOInter(vc_device->get_params()))->c_str()));
		}
	};

	// Set device params
	ENDPOINT_ASYNC("POST", "device/{domain}/{group}/{instance}/params", PostDeviceParams)
	{
		ENDPOINT_ASYNC_INIT(PostDeviceParams);

		Action act() override
		{
			return request->readBodyToStringAsync().callbackTo(&PostDeviceParams::returnResponse);
		}

		Action returnResponse(const oatpp::String& body)
		{
			std::string device_name = controller->getDeviceName(request);

			std::shared_ptr<vc::VideoCaptureDevice> vc_device = controller->vccManager->device(device_name);

			OATPP_ASSERT_HTTP(vc_device, Status::CODE_400, "Not connected to device");

			VCParamsDTOInter params = controller->apiObjectMapper->readFromString<VCParamsDTOInter>(body);
			vc_device->set_params(params);

			return _return(controller->createResponse(Status::CODE_200, "OK"));
		}
	};

	// Start record of video from device
	ENDPOINT_ASYNC("POST", "device/{domain}/{group}/{instance}/startrec", StartRecording)
	{
		ENDPOINT_ASYNC_INIT(StartRecording);

		Action act() override
		{
			std::string device_name = controller->getDeviceName(request);

			auto info = controller->vccManager->startRecording(device_name);

			OATPP_ASSERT_HTTP(info.second, Status::CODE_400, "Not connected to device");

			std::cout << "Start recording " << info.first << std::endl;

			return _return(controller->createResponse(Status::CODE_200, info.first.c_str()));
		}
	};

	// Heartbeat of Video Capture Device
	// Fronted JS application send GET request to this address for server to know that this particular device is needed
	// If no heartbeat sent for certain device in 10 seconds, thread that process this device is shut down
	ENDPOINT_ASYNC("POST", "encoder/{encodername}/heartbeat", Heartbeat)
	{
		ENDPOINT_ASYNC_INIT(Heartbeat);

		Action act() override
		{
			std::string encoder_name = request->getPathVariable("encodername");

			std::cout << "Heartbeat " << encoder_name << std::endl;

			bool res = controller->vccManager->heartBeat(encoder_name);

			OATPP_ASSERT_HTTP(res, Status::CODE_400, "Not connected to specified encoder");

			return _return(controller->createResponse(Status::CODE_200, "OK"));
		}
	};

	// Stop recording video from device
	ENDPOINT_ASYNC("GET", "record/{recordname}/stoprec", StopRecording)
	{
		ENDPOINT_ASYNC_INIT(StopRecording);

		Action act() override
		{
			std::string record_name = request->getPathVariable("recordname");

			bool res = controller->vccManager->stopRecording(record_name);

			OATPP_ASSERT_HTTP(res, Status::CODE_400, "Not connected to specified encoder");

			auto response = controller->createResponse(Status::CODE_302, "");
			response->putHeader("Location", (std::string("/record/") + record_name).c_str());

			return _return(response);
		}
	};

	ENDPOINT_ASYNC("GET", "record/{recordname}", Record)
	{
		ENDPOINT_ASYNC_INIT(Record);

		const char* pageTemplate = controller->staticFileManager->getFile("templates/record.html")->c_str();

		Action act() override
		{
			std::string record = request->getPathVariable("recordname");

			bool recordExists = controller->staticFileManager->isFileExists(std::string("playlists/records/") + record);

			OATPP_ASSERT_HTTP(recordExists, Status::CODE_400, "Record does not exists!");

			oatpp::String info = controller->staticFileManager->getFile(std::string("playlists/records/") + record + "/info.txt", true);
			int width, height;
			std::istringstream(info) >> width >> height;

			oatpp::String response = formatText(pageTemplate, std::to_string(width).c_str(), std::to_string(height).c_str(), record.c_str());

			return _return(controller->createResponse(Status::CODE_200, response->c_str()));
		}
	};

	ENDPOINT_ASYNC("GET", "recordlist", RecordList)
	{
		ENDPOINT_ASYNC_INIT(RecordList);

		Action act() override
		{
			std::vector<std::string> recordList = controller->staticFileManager->getRecordsNames();

			oatpp::List<oatpp::String> recordListObj = oatpp::List<oatpp::String>::createShared();

			for (const auto& record : recordList)
			{
				recordListObj->push_back(record.c_str());
			}

			return _return(controller->createResponse(Status::CODE_200, controller->apiObjectMapper->writeToString(recordListObj)));
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