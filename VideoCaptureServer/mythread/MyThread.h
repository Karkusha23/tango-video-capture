#ifndef MyThread_included
#define MyThread_included

#include <thread>
#include <mutex>
#include <atomic>

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
	std::atomic_bool to_exit_;

	std::mutex thread_mutex_;

	const time_t update_time_ms_;

	void run_();

};

#endif