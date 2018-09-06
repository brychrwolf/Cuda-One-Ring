#ifndef CUDAACCESS_H
#define CUDAACCESS_H

#include <vector>
#include <string>

class CudaAccess{		
		int deviceCount;
		std::vector<cudaDeviceProp> deviceProperties;
		int minWarpSize = 32;
		int minMaxBlockSize = 1024;

	public:
		CudaAccess();

		void updateDeviceProperties();

		int getDeviceCount();
		std::string getDeviceName(int d);
		int getPciBusID(int d);
		int getPciDeviceID(int d);
		int getPciDomainID(int d);
		int getMajor(int d);
		int getMinor(int d);
		int getMultiProcessorCount(int d);
		int getClockRate(int d);
		int getMemoryClockRate(int d);
		int getL2CacheSize(int d);
		size_t getSharedMemPerMultiprocessor(int d);
		size_t getTotalConstMem(int d);
		int getManagedMemory(int d);
		int getWarpSize(int d);
		int getMaxThreadsPerBlock(int d);
		int getMaxThreadsPerMultiProcessor(int d);
		int* getMaxThreadsDim(int d);
		int* getMaxGridSize(int d);
		
		int getMinWarpSize();
		int getMinMaxBlockSize();
		int getIdealBlockSizeForProblemOfSize(int n);

		void printCUDAProps();
};

#endif // CUDAACCESS_H

