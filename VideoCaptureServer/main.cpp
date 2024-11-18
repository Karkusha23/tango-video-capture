#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>

#include "AppComponent.hpp"
#include "controller/MediaController.hpp"

#include "vcclient/VCClientThread.h"

#include <experimental/filesystem>
#include <list>

// Oat++ server
void runOatServer();

// VideoCaptureClient
void runVCClient();

// Clear obsolete .ts files from playlist folder
void clearPlaylist(int remain_count);

int main()
{
	std::thread serverThread(runOatServer);
	std::thread vcclientThread(runVCClient);

	serverThread.join();
	vcclientThread.join();

	return 0;
}

void runOatServer()
{
	oatpp::base::Environment::init();

	AppComponent components;

	components.httpRouter.getObject()->addController(MediaController::createShared());

	oatpp::network::Server server(components.serverConnectionProvider.getObject(), components.serverConnectionHandler.getObject());

	OATPP_LOGI("MyApp", "Server running on port %s", components.serverConnectionProvider.getObject()->getProperty("port").getData());

	server.run();

	oatpp::base::Environment::destroy();
}

void runVCClient()
{
	vc::VCClientThread vcclientThread("CVCam/test/0", "C:\\hlsserver\\playlists\\playlist.m3u8", "http://localhost:8000/media_no_cache/playlists/", 10);

	while (true)
	{
		clearPlaylist(10);
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}

void clearPlaylist(int remain_count)
{
	static const std::experimental::filesystem::path path("C:\\hlsserver\\playlists");

	std::list<std::string> files;

	for (auto& it : std::experimental::filesystem::directory_iterator(path))
	{
		if (it.path().extension().string() == ".ts")
		{
			files.push_back(it.path().filename().string());
		}
	}

	if (files.size() <= remain_count)
	{
		return;
	}

	files.sort();

	int toDeleteCount = files.size() - remain_count;
	auto it = files.begin();
	for (int i = 0; i < toDeleteCount; ++i)
	{
		std::experimental::filesystem::remove(path / *it);
		++it;
	}
}