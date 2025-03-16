#include "VCCManager.hpp"

VCCManager::VCCManager(const char* playlists_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms) :
	MyThread(connection_heartbeat_timeout_ms), playlists_path_(playlists_path), playlist_base_url_(playlist_base_url)
{
	start();
}

std::shared_ptr<VCCManager> VCCManager::createShared(const char* playlists_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms)
{
	return std::make_shared<VCCManager>(playlists_path, playlist_base_url, connection_heartbeat_timeout_ms);
}

std::string VCCManager::formatDeviceName(const std::string& device_name, int encoder_id)
{
	std::string result = device_name;
	while (true)
	{
		auto pos = result.find("/");
		if (pos == string::npos)
		{
			break;
		}
		result.replace(pos, 1, "-");
	}
	return result + "-" + std::to_string(encoder_id);
}

std::pair<std::string, std::shared_ptr<vc::VideoCaptureDevice>> VCCManager::connectDevice(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	std::shared_ptr<vc::VCClientThread> device;

	if (device_pool_.count(device_name))
	{
		device = device_pool_[device_name];
	}
	else
	{
		try
		{
			device = std::make_shared<vc::VCClientThread>(device_name);
		}
		catch (...)
		{
			return { "", nullptr };
		}
		device_pool_.insert({ device_name, device });
	}

	auto info = device->vcDevice()->add_encoder(vc::VideoCaptureDevice::UIDisplayType::SidePanel, playlists_path_, playlist_base_url_);

	device_encoders_.insert({ info.second, { device, info.first, true } });

	return { info.second, device->vcDevice() };
}

std::shared_ptr<vc::VideoCaptureDevice> VCCManager::device(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!device_pool_.count(device_name))
	{
		return nullptr;
	}

	return device_pool_[device_name]->vcDevice();
}

std::shared_ptr<vc::VideoCaptureDevice> VCCManager::deviceByEncoderName(const std::string& device_encoder_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);
	
	if (!device_encoders_.count(device_encoder_name))
	{
		return nullptr;
	}

	device_encoders_[device_encoder_name].heartbeat = true;

	return device_encoders_[device_encoder_name].device->vcDevice();
}

bool VCCManager::disconnectDevice(const std::string& device_encoder_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!device_encoders_.count(device_encoder_name))
	{
		return false;
	}

	DeviceNode* node = &device_encoders_[device_encoder_name];
	std::shared_ptr<vc::VideoCaptureDevice> device = node->device->vcDevice();

	node->device->vcDevice()->remove_encoder(node->encoder_id);
	device_encoders_.erase(device_encoder_name);

	std::string path = playlists_path_ + "\\" + device_encoder_name;
	if (std::experimental::filesystem::exists(path))
	{
		std::experimental::filesystem::remove_all(path);
	}

	if (!device->encoder_count())
	{
		device_pool_.erase(device->deviceName());
	}

	return true;
}

bool VCCManager::isDeviceConnected(const std::string& device_encoder_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);
	return device_encoders_.count(device_encoder_name);
}

bool VCCManager::heartBeat(const std::string& device_encoder_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!device_encoders_.count(device_encoder_name))
	{
		return false;
	}

	device_encoders_[device_encoder_name].heartbeat = true;

	return true;
}

void VCCManager::update()
{
	std::lock_guard<std::mutex> lock(map_lock_);

	std::list<std::string> delete_list;
	for (auto& it : device_encoders_)
	{
		if (it.second.heartbeat)
		{
			it.second.heartbeat = false;
		}
		else
		{
			delete_list.push_back(it.first);
		}
	}

	for (auto& it : delete_list)
	{
		DeviceNode* node = &device_encoders_[it];
		std::shared_ptr<vc::VideoCaptureDevice> device = node->device->vcDevice();

		node->device->vcDevice()->remove_encoder(node->encoder_id);
		device_encoders_.erase(it);

		std::string path = playlists_path_ + "\\" + it;
		if (std::experimental::filesystem::exists(path))
		{
			std::experimental::filesystem::remove_all(path);
		}

		if (!device->encoder_count())
		{
			device_pool_.erase(device->deviceName());
		}
	}
}