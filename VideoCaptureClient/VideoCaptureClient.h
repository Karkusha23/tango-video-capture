#pragma once
#include <iostream>
#include <tango.h>

namespace vcc
{
	typedef void (*pFun)(Tango::EventData*);

	void print_device_info(std::ostream& out, Tango::DeviceProxy* device);

	enum class CameraMode : unsigned char { None, RGB, BGR, Grayscale };

	CameraMode get_device_camera_mode(Tango::DeviceProxy* device);
	int get_device_int_property(Tango::DeviceProxy* device, const std::string& name);

	class JpegCallBack final : public Tango::CallBack
	{
		pFun fun_;
	public:
		JpegCallBack(pFun fun) : Tango::CallBack(), fun_(fun) { } 
		void push_event(Tango::EventData* event_data) override;
	};
}