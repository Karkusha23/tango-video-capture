#pragma once
#include <iostream>
#include <tango.h>
#include <vc/camproc.h>

namespace vc
{
	typedef void (*pFun)(Tango::EventData*);

	void print_device_info(std::ostream& out, Tango::DeviceProxy* device);

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