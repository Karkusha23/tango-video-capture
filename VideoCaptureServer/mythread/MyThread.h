#ifndef MyThread_included
#define MyThread_included

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

// Abstract base class for thread
// Thread calls update() function every update_time_ms milliseconds

class MyThread
{
public:

	MyThread(time_t update_time_ms);
	virtual ~MyThread();

	void start();
	void stop();

	time_t updateTimeMs() const;

protected:

	virtual void update() = 0;

private:

	std::thread* thread_;
	std::atomic_bool is_started_;
	std::atomic_bool to_terminate_;

	std::mutex thread_mutex_;
	std::mutex terminate_mutex_;

	std::condition_variable terminate_cv_;

	const std::chrono::milliseconds update_time_ms_;

	void run_with_sleep_();
	void run_with_cv_();
};

#endif