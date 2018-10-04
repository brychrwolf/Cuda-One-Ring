#include <string>

#include "cudaTimer.cuh"

CudaTimer::CudaTimer(){
	name = "unnamed";
	cudaEventCreate(&startTime);
	cudaEventCreate(&stopTime);
}

CudaTimer::CudaTimer(std::string s){
	name = s;
	cudaEventCreate(&startTime);
	cudaEventCreate(&stopTime);
}

CudaTimer::~CudaTimer(){
	cudaEventDestroy(startTime);
	cudaEventDestroy(stopTime);
}

std::string CudaTimer::getName(){
	return name;
}

void CudaTimer::start(){
	cudaEventRecord(startTime);
}

void CudaTimer::stop(){
	cudaEventRecord(stopTime);
}

float CudaTimer::getElapsedTime(){
	float elapsedTime;
	cudaEventSynchronize(startTime);
	cudaEventSynchronize(stopTime);
	cudaEventElapsedTime(&elapsedTime, startTime, stopTime);
	return elapsedTime;
}

