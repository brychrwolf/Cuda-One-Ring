OBJECTS = cudaAccess.o cudaMesh.o cudaTimer.o

#TODO: main.cu no longer exists
#obj: $(OBJECTS) main.o
#	nvcc -dc $< -o $@
#	nvcc -dlink $(OBJECTS) main.o -o gpuCode.o
#	g++ $(OBJECTS) main.o gpuCode.o -L/opt/cuda/lib64 -lcudart -o obj

lib: libcudamesh.a main.cpp
	g++ -c -I. -I/opt/cuda/include main.cpp -o main.o
	g++ main.o libcudamesh.a -L/opt/cuda/lib64 -lcudart -o lib

libcudamesh.a: $(OBJECTS)
	nvcc -dlink $(OBJECTS) -o gpuCode.o
	nvcc -lib $(OBJECTS) gpuCode.o -o libcudamesh.a



cudaAccess.o: cudaAccess.cu
	nvcc -dc $< -o $@
cudaMesh.o: cudaMesh.cu
	nvcc -dc $< -o $@
cudaTimer.o: cudaTimer.cu
	nvcc -dc $< -o $@

clean:
	rm -f *.o *.a obj lib libcudamesh.a
