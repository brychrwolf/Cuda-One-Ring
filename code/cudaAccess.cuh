#ifndef CUDAACCESS_H
#define CUDAACCESS_H



class CudaAccess{
		const int WARP_SIZE = 32;
		const int MAX_BLOCK_SIZE_LT2 = 512;
		const int MAX_BLOCK_SIZE_GT2 = 1024;
		
		int deviceCount;

	public:
		CudaAccess();

		int getDeviceCount();
		int getIdealBlockSizeForProblemOfSize(int n);

		void updateDeviceCount();

		void printCUDAProps();
};

#endif // CUDAACCESS_H

