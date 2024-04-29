#include "VideoCaptureClient.h"

void vc::print_device_info(std::ostream& out, Tango::DeviceProxy* device)
{
	out << "Ping time: " << device->ping() << std::endl;
	std::cout << "Device name: " << device->name() << std::endl;

	std::vector<std::string> device_properties;
	device->get_property_list("*", device_properties);
	std::cout << "Device Properties:" << std::endl;
	for (std::string& device_property : device_properties)
	{
		std::cout << '\t' << device_property << ": ";
		Tango::DbData property_data;
		device->get_property(device_property, property_data);
		for (Tango::DbDatum& data : property_data)
		{
			std::string outstring;
			data >> outstring;
			std::cout << outstring << ' ';
		}
		std::cout << std::endl;
	}
}

vc::CameraMode vc::get_device_camera_mode(Tango::DeviceProxy* device)
{
	Tango::DbData mode_db;
	device->get_property(std::string("Mode"), mode_db);
	std::string mode;
	mode_db[0] >> mode;

	if (mode == "RGB" || mode == "rgb")
	{
		return CameraMode::RGB;
	}
	if (mode == "BGR" || mode == "bgr")
	{
		return CameraMode::BGR;
	}
	if (mode == "Grayscale" || mode == "grayscale")
	{
		return CameraMode::Grayscale;
	}

	return CameraMode::None;
}

int vc::get_device_int_property(Tango::DeviceProxy* device, const std::string& name)
{
	Tango::DbData val_db;
	device->get_property(std::string(name), val_db);
	std::string mode;
	val_db[0] >> mode;

	return std::stoi(mode);
}

void vc::JpegCallBack::push_event(Tango::EventData* event_data)
{
	fun_(event_data);
}