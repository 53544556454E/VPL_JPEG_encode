#pragma once

#include <Windows.h>

#include <chrono>
#include <sstream>

class Stopwatch
{
public:
	enum class Duration
	{
		NanoSeconds,
		MicroSeconds,
		MilliSeconds,
		Seconds
	};

	void Start(bool sleep_before_measure=true)
	{
		if (sleep_before_measure)
			Sleep(1);

		begin_time_ = std::chrono::high_resolution_clock::now();
	}

	void Stop(const char *lavel="", Stopwatch::Duration duration_type=Duration::MilliSeconds)
	{
		std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
		std::stringstream ss;

		ss << lavel << " ";

		switch (duration_type)
		{
		case Duration::NanoSeconds:
		{
			std::chrono::nanoseconds elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time_);
			ss << elapsed_time.count() << " [ns]";
		}
		break;

		case Duration::MicroSeconds:
		{
			std::chrono::microseconds elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time_);
			ss << elapsed_time.count() << " [us]";
		}
		break;

		case Duration::MilliSeconds:
		{
			std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time_);
			ss << elapsed_time.count() << " [ms]";
		}
		break;

		case Duration::Seconds:
		{
			std::chrono::seconds elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - begin_time_);
			ss << elapsed_time.count() << " [s]";
		}
		break;
		}

		OutputDebugStringA(ss.str().c_str());
	}

private:
	std::chrono::high_resolution_clock::time_point begin_time_;
};
