#include "VCClientThread.h"

namespace vc
{
	VCClientThread::VCClientThread(const char* vc_device_name, const char* playlist_path, const char* playlist_url) :
		MyThread(1000), vc_device_(vc_device_name, playlist_path, playlist_url), jpeg_callback_(&vc_device_)
	{
		event_id_ = vc_device_.device().subscribe_event(std::string("Jpeg"), Tango::CHANGE_EVENT, &jpeg_callback_, std::vector<std::string>());
		start();
	}

	VCClientThread::~VCClientThread()
	{
		stop();
		vc_device_.device().unsubscribe_event(event_id_);
	}

	VideoCaptureDevice& VCClientThread::vcDevice()
	{
		return vc_device_;
	}

	void VCClientThread::update()
	{
		vc_device_.update();
	}
}