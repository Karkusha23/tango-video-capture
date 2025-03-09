#include "VCClientThread.h"

namespace vc
{
	VCClientThread::VCClientThread(const std::string& vc_device_name, const std::string& playlist_path, const std::string& playlist_url) :
		MyThread(1000), 
		vc_device_(std::make_shared<VideoCaptureDevice>(vc_device_name, playlist_path, playlist_url, VideoCaptureDevice::UIDisplayType::SidePanel)),
		jpeg_callback_(vc_device_)
	{
		event_id_ = vc_device_->device().subscribe_event(std::string("Jpeg"), Tango::CHANGE_EVENT, &jpeg_callback_, std::vector<std::string>());
		start();
	}

	VCClientThread::~VCClientThread()
	{
		stop();
		vc_device_->device().unsubscribe_event(event_id_);
	}

	std::shared_ptr<VideoCaptureDevice> VCClientThread::vcDevice()
	{
		return vc_device_;
	}

	void VCClientThread::update()
	{
		vc_device_->update();
	}
}