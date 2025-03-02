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

std::string VCCManager::formatDeviceName(const std::string& device_name)
{
	std::string result = device_name;
	result.erase(std::remove(result.begin(), result.end(), '/'), result.end());
	return result;
}

bool VCCManager::connectDevice(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (devices_.count(device_name))
	{
		devices_[device_name].heartbeat = true;
		return true;
	}

	std::string name = formatDeviceName(device_name);
	std::string path = playlists_path_ + "\\" + name;
	std::string url = playlist_base_url_ + name + "/";
	std::shared_ptr<vc::VCClientThread> device;

	try
	{
		device = std::make_shared<vc::VCClientThread>(device_name.c_str(), (path + "\\playlist.m3u8").c_str(), url.c_str());
	}
	catch (...)
	{
		return false;
	}

	if (std::experimental::filesystem::exists(path))
	{
		std::experimental::filesystem::remove_all(path);
	}
	std::experimental::filesystem::create_directory(path);

	devices_[device_name] = { device, true };

	return true;
}

bool VCCManager::disconnectDevice(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!devices_.count(device_name))
	{
		return true;
	}

	std::string path = playlists_path_ + "\\" + formatDeviceName(device_name);

	devices_.erase(device_name);
	if (std::experimental::filesystem::exists(path))
	{
		std::experimental::filesystem::remove_all(path);
	}

	return true;
}

bool VCCManager::isDeviceConnected(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);
	return devices_.count(device_name);
}

bool VCCManager::heartBeat(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!devices_.count(device_name))
	{
		return false;
	}

	devices_[device_name].heartbeat = true;

	return true;
}

bool VCCManager::getParams(const std::string& device_name, vc::VideoCaptureDevice::Params& params)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!devices_.count(device_name))
	{
		return false;
	}

	params = devices_[device_name].device->vcDevice().get_params();
	devices_[device_name].heartbeat = true;

	return true;
}

bool VCCManager::setParams(const std::string& device_name, const vc::VideoCaptureDevice::Params& params)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!devices_.count(device_name))
	{
		return false;
	}

	devices_[device_name].device->vcDevice().set_params(params);
	devices_[device_name].heartbeat = true;

	return true;
}

void VCCManager::update()
{
	std::lock_guard<std::mutex> lock(map_lock_);

	std::list<std::string> delete_list;
	for (auto& it : devices_)
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
		std::string path = playlists_path_ + "\\" + formatDeviceName(it);
		devices_.erase(it);
		if (std::experimental::filesystem::exists(path))
		{
			std::experimental::filesystem::remove_all(path);
		}
	}
}