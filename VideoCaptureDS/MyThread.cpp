#include "MyThread.h"

namespace VideoCaptureDS_ns
{
	void* MyThread::run_undetached(void *ptr)
	{
		DEBUG_STREAM << "My thread stared!" << endl;


		// init all objects, open all connections

		while (!local_exit_)
		{
			//logic procedures

			// Sleep(100); // delay if needed
		}

		DEBUG_STREAM << "My thread stopped!" << endl;
		return 0;
	}

	MyThread::~MyThread()
	{
		// Clear all objects, close all connections
	}

	void MyThread::stop()
	{
		local_exit_ = true;
	}
}