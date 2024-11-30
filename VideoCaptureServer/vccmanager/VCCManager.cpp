#include "VCCManager.hpp"

VCCManager::VCCManager(const char* playlists_path, time_t connection_heartbeat_timeout_ms) : MyThread(connection_heartbeat_timeout_ms), playlists_path_(playlists_path)
{
	start();
}

std::shared_ptr<VCCManager> VCCManager::createShared(const char* playlists_path, time_t connection_heartbeat_timeout_ms)
{
	return std::make_shared<VCCManager>(playlists_path, connection_heartbeat_timeout_ms);
}

std::string VCCManager::formatDeviceName(const std::string& device_name)
{
	std::string result = device_name;
	result.erase(std::remove(result.begin(), result.end(), '/'), result.end());
	return result;
}

bool VCCManager::connectDevice(const std::string& device_name)
{
	{
		std::lock_guard<std::mutex> lock(map_lock_);
		if (devices_.count(device_name))
		{
			devices_[device_name].heartbeat = true;
			return true;
		}
	}

	std::string name = formatDeviceName(device_name);

	std::string path = playlists_path_ + "\\" + name;

	std::string url = std::string("http://localhost:8000/media_no_cache/playlists/") + name + "/";

	std::shared_ptr<vc::VCClientThread> device;

	try
	{
		device = std::make_shared<vc::VCClientThread>(device_name.c_str(), (path + "\\playlist.m3u8").c_str(), url.c_str(), 10);
	}
	catch (...)
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(map_lock_);

		if (std::experimental::filesystem::exists(path))
		{
			std::experimental::filesystem::remove_all(path);
		}

		std::experimental::filesystem::create_directory(path);
		devices_[device_name] = { device, true };
	}

	return true;
}

bool VCCManager::disconnectDevice(const std::string& device_name)
{
	if (!isDeviceConnected(device_name))
	{
		return true;
	}

	std::string path = playlists_path_ + "\\" + formatDeviceName(device_name);

	{
		std::lock_guard<std::mutex> lock(map_lock_);

		if (devices_[device_name].heartbeat)
		{
			return false;
		}

		devices_.erase(device_name);
		if (std::experimental::filesystem::exists(path))
		{
			std::experimental::filesystem::remove_all(path);
		}
	}

	return true;
}

bool VCCManager::isDeviceConnected(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);
	return devices_.count(device_name) > 0;
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

void VCCManager::update()
{
	std::list<std::string> delete_list;

	{
		std::lock_guard<std::mutex> lock(map_lock_);
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
	}

	for (const std::string& device_name : delete_list)
	{
		disconnectDevice(device_name);
	}
}