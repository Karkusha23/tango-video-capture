#include "VCCManager.hpp"

VCCManager::VCCManager(const char* playlist_base_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms) :
	MyThread(connection_heartbeat_timeout_ms), playlist_base_path_(playlist_base_path), playlist_base_url_(playlist_base_url)
{
	start();
}

std::shared_ptr<VCCManager> VCCManager::createShared(const char* playlist_base_path, const char* playlist_base_url, time_t connection_heartbeat_timeout_ms)
{
	return std::make_shared<VCCManager>(playlist_base_path, playlist_base_url, connection_heartbeat_timeout_ms);
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

std::pair<std::string, std::shared_ptr<vc::VideoCaptureDevice>> VCCManager::connectDeviceEncoder(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	std::shared_ptr<vc::VCClientThread> deviceThread;

	if (device_pool_.count(device_name))
	{
		deviceThread = device_pool_[device_name];
	}
	else
	{
		try
		{
			deviceThread = std::make_shared<vc::VCClientThread>(device_name);
		}
		catch (...)
		{
			return { "", nullptr };
		}
		device_pool_.insert({ device_name, deviceThread });
	}

	auto info = deviceThread->vcDevice()->add_encoder(playlist_base_path_, playlist_base_url_, false, vc::VideoCaptureDevice::UIDisplayType::SidePanel);

	device_encoders_.insert({ info.second, { deviceThread, info.first, false, true } });

	return { info.second, deviceThread->vcDevice() };
}

std::pair<std::string, std::shared_ptr<vc::VideoCaptureDevice>> VCCManager::startRecording(const std::string& device_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	std::shared_ptr<vc::VCClientThread> deviceThread;

	if (device_pool_.count(device_name))
	{
		deviceThread = device_pool_[device_name];
	}
	else
	{
		try
		{
			deviceThread = std::make_shared<vc::VCClientThread>(device_name);
		}
		catch (...)
		{
			return { "", nullptr };
		}
		device_pool_.insert({ device_name, deviceThread });
	}

	auto info = deviceThread->vcDevice()->add_encoder(playlist_base_path_ + "\\records", playlist_base_url_ + "records/", true);
	
	device_encoders_.insert({ info.second, { deviceThread, info.first, true, true } });

	return { info.second, deviceThread->vcDevice() };
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

bool VCCManager::disconnectDeviceEncoder(const std::string& device_encoder_name)
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

	if (!device->encoder_count())
	{
		device_pool_.erase(device->deviceName());
	}

	return true;
}

bool VCCManager::stopRecording(const std::string& device_encoder_name)
{
	std::lock_guard<std::mutex> lock(map_lock_);

	if (!device_encoders_.count(device_encoder_name))
	{
		return false;
	}

	DeviceNode* node = &device_encoders_[device_encoder_name];

	if (!node->isRecording)
	{
		return false;
	}

	std::shared_ptr<vc::VideoCaptureDevice> device = node->device->vcDevice();

	node->device->vcDevice()->remove_encoder(node->encoder_id);

	return true;
}

bool VCCManager::isDeviceEncoderConnected(const std::string& device_encoder_name)
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

	for (const auto& it : delete_list)
	{
		DeviceNode* node = &device_encoders_[it];
		std::shared_ptr<vc::VideoCaptureDevice> device = node->device->vcDevice();

		device->remove_encoder(node->encoder_id);
		device_encoders_.erase(it);

		if (!device->encoder_count())
		{
			device_pool_.erase(device->deviceName());
		}
	}
}