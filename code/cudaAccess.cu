#include <stdio.h>

#include <vector>
#include <string>

#include "cudaAccess.cuh"

CudaAccess::CudaAccess(){
	updateDeviceProperties();
}

void CudaAccess::updateDeviceProperties(){
	cudaError_t err = cudaGetDeviceCount(&deviceCount);
	if(err != cudaSuccess)
		deviceCount = 0;
	for(int d = 0; d < deviceCount; d++){
		cudaDeviceProp devProp;
		cudaGetDeviceProperties(&devProp, d);
		deviceProperties.push_back(devProp);
		
		if(devProp.warpSize < minWarpSize)
			minWarpSize = devProp.warpSize;
		if(devProp.maxThreadsPerBlock < minMaxBlockSize)
			minMaxBlockSize = devProp.maxThreadsPerBlock;
	}
}



int CudaAccess::getDeviceCount(){
	return deviceCount;
}

std::string CudaAccess::getDeviceName(int d){
	return deviceProperties[d].name;
}

int CudaAccess::getPciBusID(int d){
	return deviceProperties[d].pciBusID;
}

int CudaAccess::getPciDeviceID(int d){
	return deviceProperties[d].pciDeviceID;
}

int CudaAccess::getPciDomainID(int d){
	return deviceProperties[d].pciDomainID;
}

int CudaAccess::getMajor(int d){
	return deviceProperties[d].major;
}

int CudaAccess::getMinor(int d){
	return deviceProperties[d].minor;
}

int CudaAccess::getMultiProcessorCount(int d){
	return deviceProperties[d].multiProcessorCount;
}

unsigned long CudaAccess::getClockRate(int d){
	return deviceProperties[d].clockRate;
}

unsigned long CudaAccess::getMemoryClockRate(int d){
	return deviceProperties[d].memoryClockRate;
}

unsigned long CudaAccess::getL2CacheSize(int d){
	return deviceProperties[d].l2CacheSize;
}

size_t CudaAccess::getSharedMemPerMultiprocessor(int d){
	return deviceProperties[d].sharedMemPerMultiprocessor;
}

size_t CudaAccess::getTotalConstMem(int d){
	return deviceProperties[d].totalConstMem;
}

unsigned long CudaAccess::getManagedMemory(int d){
	return deviceProperties[d].managedMemory;
}

int CudaAccess::getWarpSize(int d){
	return deviceProperties[d].warpSize;
}

int CudaAccess::getMaxThreadsPerBlock(int d){
	return deviceProperties[d].maxThreadsPerBlock;
}

int CudaAccess::getMaxThreadsPerMultiProcessor(int d){
	return deviceProperties[d].maxThreadsPerMultiProcessor;
}

int* CudaAccess::getMaxThreadsDim(int d){
	return deviceProperties[d].maxThreadsDim;
}

int* CudaAccess::getMaxGridSize(int d){
	return deviceProperties[d].maxGridSize;
}



int CudaAccess::getMinWarpSize(){
	return minWarpSize;
}

int CudaAccess::getMinMaxBlockSize(){
	return minMaxBlockSize;
}

int CudaAccess::getIdealBlockSizeForProblemOfSize(int n){
	if(n > (32 * 1024) && minMaxBlockSize >= 1024)		// 32,768
		return 1024;
	else if(n > (32 * 512) && minMaxBlockSize >= 512)	// 16,384
		return 512;
	else if(n > (32 * 256) && minMaxBlockSize >= 256)	//  8,192
		return 256;
	else if(n > (32 * 128) && minMaxBlockSize >= 128)	//  4,096
		return 128;
	else if(n > (32 * 64) && minMaxBlockSize >= 64)		//  2,048
		return 64;
	else
		return minWarpSize;								// very likely 32
}



void CudaAccess::printCUDAProps(){
	printf("There are %d CUDA devices.\n", deviceCount);
	for (int d = 0; d < deviceCount; d++){
		printf("\nCUDA Device #%d\n", d);
		printf("\tName:                               %s\n",  deviceProperties[d].name);
		printf("\tPCI Bus.Device.Domain Ids:          %d.%d.%d\n",  deviceProperties[d].pciBusID,  deviceProperties[d].pciDeviceID,  deviceProperties[d].pciDomainID);
		printf("\tCUDA Capability Major/Minor version:%d.%d\n", deviceProperties[d].major, deviceProperties[d].minor);
		printf("\tNumber of multiprocessors:          %d\n",  deviceProperties[d].multiProcessorCount);
		printf("\tClock rate (kHz):                   %lu\n",  deviceProperties[d].clockRate);
		printf("\tmemoryClockRate (kHz):              %lu\n",  deviceProperties[d].memoryClockRate);
		printf("\tl2CacheSize (bytes):                %lu\n",  deviceProperties[d].l2CacheSize);
		printf("\tsharedMemPerMultiprocessor (bytes): %lu\n",  deviceProperties[d].sharedMemPerMultiprocessor); //size_t
		printf("\ttotalConstMem (bytes):              %lu\n",  deviceProperties[d].totalConstMem); //size_t
		printf("\tmanagedMemory (T/F):                %lu\n",  deviceProperties[d].managedMemory);
		printf("\twarpSize (threads):                 %d\n",  deviceProperties[d].warpSize);
		printf("\tmaxThreadsPerBlock:                 %d\n",  deviceProperties[d].maxThreadsPerBlock);
		printf("\tmaxThreadsPerMultiProcessor:        %d\n",  deviceProperties[d].maxThreadsPerMultiProcessor);
		printf("\tmaxThreadsDim[3]:                  [%d, %d, %d]\n",  deviceProperties[d].maxThreadsDim[0],  deviceProperties[d].maxThreadsDim[1],  deviceProperties[d].maxThreadsDim[2]);
		printf("\tmaxGridSize[3]:                    [%lu, %d, %d]\n",  deviceProperties[d].maxGridSize[0],  deviceProperties[d].maxGridSize[1],  deviceProperties[d].maxGridSize[2]);
	}
	printf("\nMinimum Warp Size in system:        %d\n",  minWarpSize);
	printf("Minimum Max Block Size in system:   %d\n",  minMaxBlockSize);
}

