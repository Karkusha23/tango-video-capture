#include "MyThread.h"

MyThread::MyThread(time_t update_time_ms) : thread_(nullptr), is_started_(false), to_terminate_(false), update_time_ms_(update_time_ms) {}

MyThread::~MyThread()
{
	stop();
}

void MyThread::start()
{
	std::lock_guard<std::mutex> lock(thread_mutex_);
	
	if (is_started_)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> terminateLock(terminate_mutex_);
		to_terminate_ = false;
	}

	thread_ = new std::thread(updateTimeMs() > 1000 ? &MyThread::run_with_cv_ : &MyThread::run_with_sleep_, this);
	is_started_ = true;
}

void MyThread::stop()
{
	std::lock_guard<std::mutex> lock(thread_mutex_);

	if (to_terminate_ || !thread_)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> terminateLock(terminate_mutex_);
		to_terminate_ = true;
	}
	terminate_cv_.notify_all();

	thread_->join();
	delete thread_;
	thread_ = nullptr;
	is_started_ = false;
}

time_t MyThread::updateTimeMs() const
{
	return update_time_ms_.count();
}

void MyThread::run_with_sleep_()
{
	while (!to_terminate_)
	{
		update();
		std::this_thread::sleep_for(update_time_ms_);
	}
}

void MyThread::run_with_cv_()
{
	std::unique_lock<std::mutex> lock(terminate_mutex_);

	while (!to_terminate_)
	{
		update();
		terminate_cv_.wait_for(lock, update_time_ms_, [this]{ return (bool)to_terminate_; });
	}
}

