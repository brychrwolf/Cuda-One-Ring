OBJECTS = cudaAccess.o cudaMesh.o cudaTimer.o

main: $(OBJECTS) main.o
	nvcc $(OBJECTS) main.o -o main

obj: $(OBJECTS) main.o
	nvcc -dlink $(OBJECTS) main.o -o gpuCode.o
	#TODO: cuda located here on Manjaro, elsewhere on others?
	g++ $(OBJECTS) main.o gpuCode.o -L/opt/cuda/lib64 -lcudart -o obj

lib: $(OBJECTS) main.cpp
	nvcc -dlink $(OBJECTS) -o gpuCode.o
	nvcc -lib $(OBJECTS) gpuCode.o -o libcudamesh.a
	g++ -c -I. -I/opt/cuda/include main.cpp -o main.o
	g++ main.o libcudamesh.a -L/opt/cuda/lib64 -lcudart -o lib

%.o: %.cu
	nvcc -dc $< -o $@

clean:
	rm -f *.o *.a main obj lib libcudamesh.so
