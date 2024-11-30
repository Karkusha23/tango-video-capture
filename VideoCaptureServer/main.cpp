#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>

#include "AppComponent.hpp"
#include "controller/MyApiController.hpp"

// Running Oat++ server

int main()
{
	oatpp::base::Environment::init();

	AppComponent components;

	components.httpRouter.getObject()->addController(MyApiController::createShared());

	oatpp::network::Server server(components.serverConnectionProvider.getObject(), components.serverConnectionHandler.getObject());

	OATPP_LOGI("MyApp", "Server running on port %s", components.serverConnectionProvider.getObject()->getProperty("port").getData());

	server.run();

	oatpp::base::Environment::destroy();

	return 0;
}