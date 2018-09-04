OBJECTS = main.o cudaAccess.o cudaMesh.o cudaTimer.o

main: $(OBJECTS)
	nvcc $(OBJECTS) -o main

lib: $(OBJECTS)
	nvcc -dlink $(OBJECTS) -o gpuCode.o
	#TODO: here on Manjaro, elsewhere on others?
	g++ gpuCode.o $(OBJECTS) -L/opt/cuda/lib64 -lcudart -o main-lib

#Compile all cu files into objects
%.o: %.cu
	nvcc -dc $< -o $@

clean:
	rm -f *.o *.a main main-lib
