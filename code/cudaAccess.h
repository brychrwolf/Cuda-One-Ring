#ifndef CUDAACCESS_H
#define CUDAACCESS_H

class CudaAccess{
	public:
		CudaAccess();
		
		// Print out properties of found CUDA devices
		void printCUDAProps(int devCount);
};

#endif // CUDAACCESS_H

