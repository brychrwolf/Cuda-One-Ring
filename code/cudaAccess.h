#ifndef CUDAACCESS_H
#define CUDAACCESS_H

class CudaAccess{
		int deviceCount;

	public:
		CudaAccess();

		void updateDeviceCount();
		int getDeviceCount();
		void printCUDAProps();
};

#endif // CUDAACCESS_H

