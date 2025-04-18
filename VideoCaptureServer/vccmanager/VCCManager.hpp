#ifndef VCCManager_hpp_included
#define VCCManager_hpp_included

#include <memory>
#include <set>
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
	// connection_heartbeat_timeout_ms - number of milliseconds that has to be passed without heartbeat from web client to shut down VideoEncoderThread
	VCCManager(const char* playlist_base_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms);

	static std::shared_ptr<VCCManager> createShared(const char* playlist_base_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms);

	// Replaces '/' symbols with '-' symbols in Tango device name
	static std::string formatDeviceName(const std::string& device_name, int encoder_id);

	std::pair<std::string, std::shared_ptr<vc::VideoCaptureDevice>> connectDeviceEncoder(const std::string& device_name);
	std::pair<std::string, std::shared_ptr<vc::VideoCaptureDevice>> startRecording(const std::string& device_name);
	std::shared_ptr<vc::VideoCaptureDevice> device(const std::string& device_name);
	std::shared_ptr<vc::VideoCaptureDevice> deviceByEncoderName(const std::string& device_encoder_name);

	bool disconnectDeviceEncoder(const std::string& device_encoder_name);
	bool stopRecording(const std::string& device_encoder_name);
	bool isDeviceEncoderConnected(const std::string& device_encoder_name);
	bool heartBeat(const std::string& device_encoder_name);

private:

	void update() override;

	std::string playlist_base_path_;
	std::string playlist_base_url_;

	struct DeviceNode
	{
		std::shared_ptr<vc::VCClientThread> device;
		int encoder_id;
		bool isRecording;
		bool heartbeat;
	};

	std::unordered_map<std::string, DeviceNode> device_encoders_;
	std::unordered_map<std::string, std::shared_ptr<vc::VCClientThread>> device_pool_;

	std::mutex map_lock_;
};

#endif