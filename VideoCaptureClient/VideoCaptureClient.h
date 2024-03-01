#pragma once
#include <iostream>
#include <tango.h>

namespace vcc
{
	void print_device_info(std::ostream& out, Tango::DeviceProxy* device);

	enum class CameraMode : unsigned char { None, RGB, BGR, Grayscale };

	CameraMode get_device_camera_mode(Tango::DeviceProxy* device);
}