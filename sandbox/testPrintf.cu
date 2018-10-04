#include <stdio.h>

__global__ 
void testKernel(int val){
	int blockIndex = blockIdx.y*gridDim.x+blockIdx.x;
	int threadIndex = threadIdx.z*blockDim.x*blockDim.y+threadIdx.y*blockDim.x+threadIdx.x;
	double l2n_d = 1.9;
    printf("[%d, %d]:\t\tValue is:%g\n",\
            blockIndex,\
            threadIndex,\
            l2n_d);
}

int main(){
	dim3 dimGrid(2, 2);
	dim3 dimBlock(2, 2, 2);
	testKernel<<<dimGrid, dimBlock>>>(10);
	cudaDeviceSynchronize();
	return EXIT_SUCCESS;
}
