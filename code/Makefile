main:
	nvcc main.cu cudaAccess.cu cudaMesh.cu cudaTimer.cu -o main

lib:
	nvcc -c		main.cu			-o main.o
	
	nvcc -c		cudaAccess.cu	-o cudaAccess.o
	nvcc -c		cudaMesh.cu		-o cudaMesh.o
	nvcc -c		cudaTimer.cu	-o cudaTimer.o

	ar rcs 	libcuda.a		cudaAccess.o cudaMesh.o cudaTimer.o

	nvcc	main.o		-lcuda		-o main-lib

clean:
	rm -f *.o *.a *.linkinfo a.out *~ main main-lib
