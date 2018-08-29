#include <stdio.h>
#include "cudaAccess.cuh"

CudaAccess::CudaAccess(){
	//TODO: Maintain properties for each device
	updateDeviceCount();
}

int CudaAccess::getDeviceCount(){
	printf("deviceCount %d\n", deviceCount);
	return deviceCount;
}

int CudaAccess::getIdealBlockSizeForProblemOfSize(int n){
	//TODO: research required calculations for optimization
	if(n > (1024 * 1024))		// 1,048,576
		return 1024;
	else if(n > (512 * 512))	//   262,144
		return 512;
	else if(n > (256 * 256))	//    65,536
		return 256;
	else if(n > (128 * 128))	//    16,384
		return 128;
	else if(n > (64 * 64))		//     4,096
		return 64;
	else
		return 32;
}



void CudaAccess::updateDeviceCount(){
	cudaError_t err = cudaGetDeviceCount(&deviceCount);
	if(err != cudaSuccess)
		deviceCount = 0;
}



void CudaAccess::printCUDAProps(){
	printf("There are %d CUDA devices.\n", deviceCount);

	// Iterate through devices
	for (int i = 0; i < deviceCount; ++i)
	{
		// Get device properties
		printf("\nCUDA Device #%d\n", i);
		cudaDeviceProp devProp;
		cudaGetDeviceProperties(&devProp, i);

    	printf("Name:                          %s\n",  devProp.name);
    	printf("Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
    	printf("Clock rate:                    %d\n",  devProp.clockRate);
    	printf("Total constant memory:         %u\n",  devProp.totalConstMem);
		printf("CUDA Capability Major/Minor version number:    %d.%d\n", devProp.major, devProp.minor);

	}
}

