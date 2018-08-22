#include <stdio.h>
#include "cudaAccess.h"

CudaAccess::CudaAccess(){
	printf("CudaAccess Constructed");	
}

void CudaAccess::printCUDAProps(int devCount){
	printf("There are %d CUDA devices.\n", devCount);

	// Iterate through devices
	for (int i = 0; i < devCount; ++i)
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

