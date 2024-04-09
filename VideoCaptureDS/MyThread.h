#ifndef MyThreadIncluded
#define MyThreadIncluded

#include <tango.h>

namespace VideoCaptureDS_ns
{
	class MyThread : public omni_thread, public Tango::LogAdapter
	{
	public:
		MyThread(TANGO_BASE_CLASS* dev) : omni_thread(), Tango::LogAdapter(dev)
		{
				start_undetached();
		}
		~MyThread();
		void* run_undetached(void*);
		void stop();
	private:
		TANGO_BASE_CLASS* ds_;
		bool local_exit_;
	};
}	//	End of namespace
#endif  