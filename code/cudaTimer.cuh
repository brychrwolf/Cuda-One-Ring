#ifndef CUDATIMER_H
#define CUDATIMER_H

#include <string>

class CudaTimer{
	std::string name;
	cudaEvent_t startTime, stopTime;

	public:
		CudaTimer();
		CudaTimer(std::string s);
		~CudaTimer();

		std::string getName();
		void start();
		void stop();
		float getElapsedTime();
};

#endif // CUDATIMER_H

