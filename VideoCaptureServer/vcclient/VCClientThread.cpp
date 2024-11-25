#include "VCClientThread.h"

namespace vc
{
	VCClientThread::VCClientThread(const char* vc_device_name, const char* playlist_path, const char* playlist_url, int framerate) :
		MyThread(1000 / framerate), vc_device_(vc_device_name, playlist_path, playlist_url, framerate, false), jpeg_callback_(&vc_device_), playlist_path_(playlist_path), clear_counter_(0)
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

		/*clear_counter_ += updateTimeMs();
		if (clear_counter_ > 20000)
		{
			clear_counter_ = 0;
			std::thread thread(&VCClientThread::clearPlaylist, this, 10);
			thread.detach();
		}*/
	}

	void VCClientThread::clearPlaylist(int remain_count)
	{
		static const std::experimental::filesystem::path path(playlist_path_);

		if (remain_count <= 0)
		{
			return;
		}

		std::cout << "Clearing playlist" << std::endl;

		std::list<std::string> files;

		for (auto& it : std::experimental::filesystem::directory_iterator(path))
		{
			if (it.path().extension().string() == ".ts")
			{
				files.push_back(it.path().filename().string());
			}
		}

		if (files.size() <= remain_count)
		{
			return;
		}

		files.sort();

		int toDeleteCount = files.size() - remain_count;
		auto it = files.begin();
		for (int i = 0; i < toDeleteCount; ++i)
		{
			std::experimental::filesystem::remove(path / *it);
			++it;
		}
	}
}