#ifndef VCCManager_hpp_included
#define VCCManager_hpp_included

#include <memory>
#include <unordered_map>
#include <mutex>

#include "../vcclient/VCClientThread.h"

class VCCManager : public MyThread
{
public:

	VCCManager(const char* playlists_path, time_t connection_heartbeat_timeout_ms);

	static std::shared_ptr<VCCManager> createShared(const char* playlists_path, time_t connection_heartbeat_timeout_ms);

	static std::string formatDeviceName(const std::string& device_name);

	bool connectDevice(const std::string& device_name);
	bool disconnectDevice(const std::string& device_name);
	bool isDeviceConnected(const std::string& device_name);
	bool heartBeat(const std::string& device_name);

private:

	void update() override;

	std::string playlists_path_;

	struct DeviceNode
	{
		std::shared_ptr<vc::VCClientThread> device;
		bool heartbeat;
	};

	std::unordered_map<std::string, DeviceNode> devices_;

	std::mutex map_lock_;
};

#endif