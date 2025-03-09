#ifndef VCCManager_hpp_included
#define VCCManager_hpp_included

#include <memory>
#include <unordered_map>
#include <mutex>

#include "../vcclient/VCClientThread.h"

// Manager of Video Capture Clients
// Manages all instances of VCClientThread
// Thus, Oat++ can provide connection to multiple VideoCapture devices simultaneously

class VCCManager : public MyThread
{
public:

	// playlists_path - path of folder that stores all the playlists from VideoCapture clients
	// connection_heartbeat_timeout_ms - number of milliseconds that has to be passed without heartbeat from web client to shut down VCClientThread
	VCCManager(const char* playlists_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms);

	static std::shared_ptr<VCCManager> createShared(const char* playlists_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms);

	// Remove '/' symbols from Tango device name
	static std::string formatDeviceName(const std::string& device_name);

	std::shared_ptr<vc::VideoCaptureDevice> connectDevice(const std::string& device_name);
	std::shared_ptr<vc::VideoCaptureDevice> device(const std::string& device_name);

	bool disconnectDevice(const std::string& device_name);
	bool isDeviceConnected(const std::string& device_name);
	bool heartBeat(const std::string& device_name);

private:

	void update() override;

	std::string playlists_path_;
	std::string playlist_base_url_;

	struct DeviceNode
	{
		std::shared_ptr<vc::VCClientThread> device;
		bool heartbeat;
	};

	std::unordered_map<std::string, DeviceNode> devices_;

	std::mutex map_lock_;
};

#endif