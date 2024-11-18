#include "MyThread.h"

MyThread::MyThread(time_t update_time_ms) : thread_(nullptr), is_started_(false), to_exit_(false), update_time_ms_(update_time_ms) {}

MyThread::~MyThread()
{
	stop();
}

void MyThread::start()
{
	if (is_started_)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(thread_mutex_);

		to_exit_ = false;
		thread_ = new std::thread(&MyThread::run_, this);
		is_started_ = true;
	}
}

void MyThread::stop()
{
	if (to_exit_ || !thread_)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(thread_mutex_);

		to_exit_ = true;
		thread_->join();
		delete thread_;
		thread_ = nullptr;
		is_started_ = false;
	}
}

void MyThread::run_()
{
	while (!to_exit_)
	{
		update();
		std::this_thread::sleep_for(std::chrono::milliseconds(update_time_ms_));
	}
}

