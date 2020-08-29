#pragma once
#include <chrono>
#include <iostream>

class hyChrono
{
	std::chrono::steady_clock::time_point StartTime;
public:
	
	hyChrono() : StartTime() {}
	~hyChrono() {}

	inline void Reset()
	{
		 StartTime = std::chrono::steady_clock::now();
	}

	/* Returns time elapsed in milliseconds. */
	inline double GetTime()
	{
		//std::cout << "T: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - StartTime).count() / 1000.0 << std::endl;
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - StartTime).count() / 1000.0;
	}



};