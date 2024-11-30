#ifndef VCClientThread_included
#define VCClientThread_included

#include <tango.h>

#include <experimental/filesystem>

#include "VideoCaptureClient.h"
#include "../mythread/MyThread.h"

// Thread wrapper for Video Capture Client
// Contains Video Capture Client instance itself and callback for image change event

namespace vc
{
	class VCClientThread : public MyThread
	{
	public:

		VCClientThread(const char* vc_device_name, const char* playlist_path, const char* playlist_url);
		virtual ~VCClientThread();

		VideoCaptureDevice& vcDevice();

	private:

		void update() override;

		VideoCaptureDevice vc_device_;
		JpegCallBack jpeg_callback_;

		int event_id_;
	};
}

#endif