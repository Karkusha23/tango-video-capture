#ifndef AppComponent_hpp_included
#define AppComponent_hpp_included

#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/web/server/AsyncHttpConnectionHandler.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include "filemanager/StaticFileManager.hpp"
#include "vccmanager/VCCManager.hpp"

// Defining Oat++ app components

class AppComponent
{
public:

	// Server connection provider component
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([]
	{ 
		return oatpp::network::tcp::server::ConnectionProvider::createShared({ "localhost", 8000, oatpp::network::Address::IP_4 }); 
	}());

	// Http router component
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([]
	{
		return oatpp::web::server::HttpRouter::createShared();
	}());

	// Async executor component
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([]
	{
		return std::make_shared<oatpp::async::Executor>(4, 1, 1);
	}());

	// Server connection handler component
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([]
	{
		OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
		OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
		return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
	}());

	// Object mapper component
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([]
	{
		auto serializerConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
		auto deserializerConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();
		deserializerConfig->allowUnknownFields = false;
		return oatpp::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
	}());

	// File manager component
	OATPP_CREATE_COMPONENT(std::shared_ptr<StaticFileManager>, staticFileManager)([]
	{
		return StaticFileManager::createShared("C:\\hlsserver");
	}());

	// Video Capture Client manager component
	OATPP_CREATE_COMPONENT(std::shared_ptr<VCCManager>, vccManager)([]
	{
		return VCCManager::createShared("C:\\hlsserver\\playlists", "http://localhost:8000/media_no_cache/playlists/", 10000);
	}());
};

#endif