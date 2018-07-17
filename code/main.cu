#include <stdio.h>
#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>
#include <array>
#include <vector>
#include <map>

// to engage GPUs when installed in hybrid system, run as 
// optirun ./main

typedef std::array<double, 3> vertex;
typedef std::array<int, 3> face;

vertex scale(vertex v, double scalar);
vertex combine(vertex v1, vertex v2);
double l2norm(const vertex vi);
double l2norm_diff(const vertex vi, const vertex v0);

void printCUDAProps(int devCount);
void loadMesh_syntheticH(vertex vertices[], double featureVectors[], face faces[]);
void flattenMesh(int numVertices, vertex vertices[], double flat_vertices[], double featureVectors[], int numFaces, face faces[], int flat_faces[]);
void printMesh(int numVertices, vertex vertices[], double featureVectors[], int numFaces, face faces[]);

__global__ void buildLookupTables(int numFaces, int* flat_faces, int* facesOfVertices, int* adjacentVertices);
__global__ void getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* flat_vertices, double* edgeLengths);
__device__ int getV0FromRunLength(int numVertices, int av, int* adjacentVertices_runLength);
__device__ double cuda_l2norm_diff(int vi, int v0, double* flat_vertices);
__global__ void getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* flat_vertices, double* edgeLengths, double* minEdgeLength);
__global__ void getFPrimes(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* featureVectors, double* minEdgeLength, double* flat_vertices, double* f_primes);
__global__ void getCircleSectors(int numVertices, int* facesOfVertices_runLength, int* flat_facesOfVertices, int* flat_faces, double* edgeLengths);
__device__ void getViAndVip1FromV0andFi(int v0, int fi, int* flat_faces, int& vi, int& vip1);

int main(){
	/***************************************************************/
	std::cout << std::endl << "****** Initialize CUDA." << std::endl;
	/***************************************************************/
	int devCount;
	cudaGetDeviceCount(&devCount);
	printf("CUDA Device Query...\n");
	if(devCount <= 0)
		std::cout << "No CUDA devices found." << std::endl;
	else
		printCUDAProps(devCount);
	int blockSize;
	int numBlocks;
	/******************************************************************/
	std::cout << "****** CUDA Initialized." << std::endl;
	/******************************************************************/



	/******************************************************************/
	std::cout << std::endl << "****** Begin Loading Mesh." << std::endl;
	/******************************************************************/
	const int numVertices = 22;
	vertex *vertices;
	double *flat_vertices;
	double *featureVectors;
	cudaMallocManaged(&vertices, numVertices*sizeof(vertex));
	cudaMallocManaged(&flat_vertices, 3*numVertices*sizeof(double));
	cudaMallocManaged(&featureVectors, numVertices*sizeof(double));
	
	const int numFaces = 36;
	face *faces;
	int *flat_faces;
	cudaMallocManaged(&faces, numFaces*sizeof(face));
	cudaMallocManaged(&flat_faces, 3*numFaces*sizeof(int));
	
	loadMesh_syntheticH(vertices, featureVectors, faces);
	//printMesh(numVertices, vertices, featureVectors, numFaces, faces);
	flattenMesh(numVertices, vertices, flat_vertices, featureVectors, numFaces, faces, flat_faces);
	/***************************************************/
	std::cout << "****** Finished Loading." << std::endl;
	/***************************************************/


	
	/***********************************************************************/
	std::cout << std::endl << "****** Begin Building Tables..." << std::endl;
	/***********************************************************************/
	std::cout << "Building table of faces by vertex, " << std::endl;
	std::cout << "and table of adjacent vertices by vertex..." << std::endl;
	std::set<int> facesOfVertices[numVertices] = {};
	std::set<int> adjacentVertices[numVertices] = {};
	
	//int numCombos = numFaces * numVertices;
	//int blockSize = 256;
	//int numBlocks = (numCombos + blockSize - 1) / blockSize;
	//buildLookupTables<<<numBlocks, blockSize>>>(numFaces, faces, facesOfVertices, adjacentVertices);

	std::cout << "Iterating over each vertex as v..." << std::endl;
	std::cout << "as well as Iterating over each face as f..." << std::endl;
	//TODO: Determine if this way is optimal:
	//	edges saved twice, once in each direction, but enables use of runLength array...
	for(int v = 0; v < numVertices; v++){
		for(int f = 0; f < numFaces; f++){
			if(faces[f][0] == v){
				facesOfVertices[v].insert(f);
				adjacentVertices[v].insert(faces[f][1]);
				adjacentVertices[v].insert(faces[f][2]);
			}			
			else if(faces[f][1] == v){
				facesOfVertices[v].insert(f);
				adjacentVertices[v].insert(faces[f][0]);
				adjacentVertices[v].insert(faces[f][2]);
			}			
			else if(faces[f][2] == v){
				facesOfVertices[v].insert(f);
				adjacentVertices[v].insert(faces[f][0]);
				adjacentVertices[v].insert(faces[f][1]);
			}
		}
	}
	
	// Determine runlengths of adjacentVertices and facesofVertices
	int* adjacentVertices_runLength;
	int* facesOfVertices_runLength;
	cudaMallocManaged(&adjacentVertices_runLength, numVertices*sizeof(int));
	cudaMallocManaged(&facesOfVertices_runLength,  numVertices*sizeof(int));
	adjacentVertices_runLength[0] = adjacentVertices[0].size();
	facesOfVertices_runLength[0]  = facesOfVertices[0].size();
	//std::cout << "adjacentVertices_runLength[" << 0 << "] " << adjacentVertices_runLength[0] << std::endl;
	//std::cout << "facesOfVertices_runLength[" << 0 << "] " << facesOfVertices_runLength[0] << std::endl;
	std::cout << "Iterating over each vertex as v0..." << std::endl;
	for(int v0 = 0+1; v0 < numVertices; v0++){
		adjacentVertices_runLength[v0] = adjacentVertices_runLength[v0-1] + adjacentVertices[v0].size();
		facesOfVertices_runLength[v0]  = facesOfVertices_runLength[v0-1]  + facesOfVertices[v0].size();
		//std::cout << "adjacentVertices_runLength[" << v0 << "] " << adjacentVertices_runLength[v0] << std::endl;
		//std::cout << "facesOfVertices_runLength[" << v0 << "] " << facesOfVertices_runLength[v0] << std::endl;
	}
	
	// Flatten adjacentVerticies and facesOfVertices
	int numAdjacentVertices = adjacentVertices_runLength[numVertices-1];
	int numFacesOfVertices  = facesOfVertices_runLength[numVertices-1];
	int* flat_adjacentVertices;
	int* flat_facesOfVertices;
	cudaMallocManaged(&flat_adjacentVertices, numAdjacentVertices*sizeof(int));
	cudaMallocManaged(&flat_facesOfVertices, numFacesOfVertices*sizeof(int));
	int r = 0;
	int s = 0;
	std::cout << "Iterating over each vertex as v0..." << std::endl;
	for(int v0 = 0; v0 < numVertices; v0++){
		for(std::set<int>::iterator vi_iter = adjacentVertices[v0].begin(); vi_iter != adjacentVertices[v0].end(); vi_iter++){
			int vi = *vi_iter;
			flat_adjacentVertices[r] = vi;
			//std::cout << "flat_adjacentVertices[" << r << "] " << flat_adjacentVertices[r] << std::endl;
			r++;
		}
		for(std::set<int>::iterator vi_iter = facesOfVertices[v0].begin(); vi_iter != facesOfVertices[v0].end(); vi_iter++){
			int vi = *vi_iter;
			flat_facesOfVertices[s] = vi;
			//std::cout << "flat_facesOfVertices[" << s << "] " << flat_facesOfVertices[s] << std::endl;
			s++;
		}
	}
	
	// Precalculate Edge Lengths
	double* edgeLengths;
	cudaMallocManaged(&edgeLengths, numAdjacentVertices*sizeof(double));
	blockSize = 32;
	numBlocks = max(1, numAdjacentVertices / blockSize);
	std::cout << "getEdgeLengths<<<" << numBlocks << ", " << blockSize <<">>(" << numAdjacentVertices << ")" << std::endl;
	getEdgeLengths<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, flat_vertices, edgeLengths);
	cudaDeviceSynchronize();	//wait for GPU to finish before accessing on host
	/***********************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/***********************************************************/



	/******************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/******************************************************************/
	std::cout << "Calculating minimum edge length among adjacent vertices..." << std::endl;
	double* minEdgeLength;
	cudaMallocManaged(&minEdgeLength, numVertices*sizeof(double));
	blockSize = 8;
	numBlocks = max(1, numVertices / blockSize);
	std::cout << "getMinEdgeLength<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	getMinEdgeLength<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, adjacentVertices_runLength, flat_vertices, edgeLengths, minEdgeLength);
	cudaDeviceSynchronize();

	std::cout << std::endl << "Calculating f', weighted mean f0 and fi by distance..." << std::endl;
	double* f_primes;
	cudaMallocManaged(&f_primes, numAdjacentVertices*sizeof(double));
	blockSize = 32;
	numBlocks = max(1, numAdjacentVertices / blockSize);
	std::cout << "getFPrimes<<<" << numBlocks << ", " << blockSize << ">>(" << numAdjacentVertices << ")" << std::endl;
	getFPrimes<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, featureVectors, minEdgeLength, flat_vertices, f_primes);
	cudaDeviceSynchronize();
	
	std::cout << std::endl << "Calculating circle_sectors..." << std::endl;
	double* circleSectors;
	cudaMallocManaged(&circleSectors, numVertices*sizeof(double));
	blockSize = 8;
	numBlocks = max(1, numVertices / blockSize);
	std::cout << "getCircleSectors<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
//	getMinEdgeLength(numAdjacentVertices, numVertices, adjacentVertices_runLength, flat_vertices, edgeLengths, minEdgeLength);
	getCircleSectors<<<numBlocks, blockSize>>>(numVertices, facesOfVertices_runLength, flat_facesOfVertices, flat_faces, edgeLengths);
	cudaDeviceSynchronize();
	/******************************************************************/
	std::cout << "****** Finished Calculating." << std::endl;
	/******************************************************************/
}



vertex scale(vertex v, double scalar){
	return {v[0]*scalar,
			v[1]*scalar, 
			v[2]*scalar};
}

vertex combine(vertex v1, vertex v2){
	return {v1[0] + v2[0],
			v1[1] + v2[1],
			v1[2] + v2[2]};
}

double l2norm(const vertex vi){
	return sqrt(vi[0]*vi[0]
			  + vi[1]*vi[1]
			  + vi[2]*vi[2]);
}

double l2norm_diff(const vertex vi, const vertex v0){
	return sqrt((vi[0] - v0[0])*(vi[0] - v0[0])
			  + (vi[1] - v0[1])*(vi[1] - v0[1])
			  + (vi[2] - v0[2])*(vi[2] - v0[2]));
}

void printCUDAProps(int devCount){
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

void loadMesh_syntheticH(
	vertex vertices[], 
	double featureVectors[], 
	face faces[]
){
	std::cout << "Loading Vertices..." << std::endl;
	vertices[0]  = { 0,  0,  0};
	vertices[1]  = { 2,  0,  0};
	vertices[2]  = {12,  0,  0};
	vertices[3]  = {14,  0,  0};
	vertices[4]  = {14, 20,  0};
	vertices[5]  = {12, 20,  0};
	vertices[6]  = { 2, 20,  0};
	vertices[7]  = { 0, 20,  0};
	vertices[8]  = { 1,  1, -1};
	vertices[9]  = {13,  1, -1};
	vertices[10] = {13, 19, -1};
	vertices[11] = { 1, 19, -1};
	vertices[12] = { 2, 10,  0};
	vertices[13] = {12, 10,  0};
	vertices[14] = {12, 12,  0};
	vertices[15] = { 2, 12,  0};
	vertices[16] = { 1, 11, -1};
	vertices[17] = {13, 11, -1};
	vertices[18] = {-2, -2,  0};
	vertices[19] = {16, -2,  0};
	vertices[20] = {16, 22,  0};
	vertices[21] = {-2, 22,  0};	
	
	/*std::cout << std::endl << "Generating Random Feature Vectors..." << std::endl;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);
	double featureVectors[numVertices] = {};
	for(int i = 0; i < numVertices; i++){
		featureVectors[i] = dis(gen);
		std::cout << "featureVector [" << i << "] = " << featureVectors[i] << std::endl;
	}*/
	
	std::cout << "Loading Feature Vectors..." << std::endl;
	featureVectors[0]  = -0.373397;
	featureVectors[1]  =  0.645161;
	featureVectors[2]  =  0.797587;
	featureVectors[3]  = -0.520541;
	featureVectors[4]  = -0.114591;
	featureVectors[5]  =  0.788363;
	featureVectors[6]  = -0.936573;
	featureVectors[7]  = -0.699675;
	featureVectors[8]  = -0.139383;
	featureVectors[9]  =  0.152594;
	featureVectors[10] = -0.976301;
	featureVectors[11] =  0.288434;
	featureVectors[12] = -0.212369;
	featureVectors[13] =  0.722184;
	featureVectors[14] =  0.154177;
	featureVectors[15] =  0.510287;
	featureVectors[16] =  0.725236;
	featureVectors[17] =  0.992415;
	featureVectors[18] =  0.582556;
	featureVectors[19] =  0.272700;
	featureVectors[20] = -0.692900;
	featureVectors[21] =  0.405410;
	
	std::cout << "Loading Faces..." << std::endl;
	faces[0]  = { 0,  1,  8};
	faces[1]  = { 1, 16,  8};
	faces[2]  = { 1, 12, 16};
	faces[3]  = {12, 13, 16};
	faces[4]  = {13, 17, 16};
	faces[5]  = { 9, 17, 13};
	faces[6]  = { 2,  9, 13};
	faces[7]  = { 2,  3,  9};
	faces[8]  = { 3, 10,  9};
	faces[9]  = { 3,  4, 10};
	faces[10] = { 4,  5, 10};
	faces[11] = { 5, 17, 10};
	faces[12] = { 5, 14, 17};
	faces[13] = {14, 15, 17};
	faces[14] = {15, 16, 17};
	faces[15] = {11, 16, 15};
	faces[16] = { 6, 11, 15};
	faces[17] = { 6,  7, 11};
	faces[18] = { 7,  8, 11};
	faces[19] = { 0,  8,  7};
	faces[20] = { 0, 18,  1};
	faces[21] = { 1, 18, 19};
	faces[22] = { 1, 19,  2};
	faces[23] = { 2, 19,  3};
	faces[24] = { 3, 19,  4};
	faces[25] = { 4, 19, 20};
	faces[26] = { 4, 20,  5};
	faces[27] = { 5, 20, 21};
	faces[28] = { 5, 21,  6};
	faces[29] = { 6, 21,  7};
	faces[30] = { 0,  7, 21};
	faces[31] = { 0, 21, 18};
	faces[32] = { 1,  2, 12};
	faces[33] = { 2, 13, 12};
	faces[34] = { 5,  6, 14};
	faces[35] = { 6, 15, 14};
}

void flattenMesh(
	int numVertices, 
	vertex vertices[],
	double flat_vertices[],
	double featureVectors[],
	int numFaces, 
	face faces[],
	int flat_faces[]
){
	for(int v = 0; v < numVertices; v++){
		flat_vertices[(v*3)+0] = vertices[v][0];
		flat_vertices[(v*3)+1] = vertices[v][1];
		flat_vertices[(v*3)+2] = vertices[v][2];
		//std::cout << "flat_vertices[" << v << "*3+{0,1,2}] {" << flat_vertices[(v*3)+0] << ", " << flat_vertices[(v*3)+1] << ", " << flat_vertices[(v*3)+2] << "}" << std::endl;
	}
	for(int f = 0; f < numFaces; f++){
		flat_faces[(f*3)+0] = faces[f][0];
		flat_faces[(f*3)+1] = faces[f][1];
		flat_faces[(f*3)+2] = faces[f][2];
		std::cout << "flat_faces[" << f << "*3+{0,1,2}] {" << flat_faces[(f*3)+0] << ", " << flat_faces[(f*3)+1] << ", " << flat_faces[(f*3)+2] << "}" << std::endl;
	}
}

void printMesh(
	int numVertices, 
	vertex vertices[], 
	double featureVectors[], 
	int numFaces, 
	face faces[]
){
	for(int v = 0; v < numVertices; v++){
		std::cout << "vertices[" << v << "] = ";
		for(int i=0; i < 3; i++){
			if(i > 0){
				std::cout << ", ";
			}
			std::cout << vertices[v][i];
		}
		std::cout << " featureVector = " << featureVectors[v] << std::endl;
	}
	for(int f = 0; f < numFaces; f++)
		std::cout << f << " = {" << faces[f][0] << ", " << faces[f][1] << ", " << faces[f][2] << "}" <<std::endl;
}


__global__
void buildLookupTables(int numFaces, int* flat_faces, int* facesOfVertices, int* adjacentVertices){
	//int index = blockIdx.x * blockDim.x + threadIdx.x;
	//int stride = blockDim.x * gridDim.x;

	//int v = index / numFaces;
	//int f = index % numFaces;
}

__global__
void getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* flat_vertices, double* edgeLengths){
	//int blockIndex = blockIdx.x; //0-2
	//int local_threadIndex = threadIdx.x; //0-31
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	int stride = blockDim.x * gridDim.x; //32*3 = 96

	// Use all availble threads to do all numAdjacentVertices
	for(int av = global_threadIndex; av < numAdjacentVertices; av += stride){
		int vi = flat_adjacentVertices[av];
		int v0 = getV0FromRunLength(numVertices, av, adjacentVertices_runLength);
		edgeLengths[av] = cuda_l2norm_diff(vi, v0, flat_vertices);
		//printf("edgeLength[%d]\t(v0 %d, vi %d)\t%g\n", av, v0, vi, edgeLengths[av]);
	}
}

__device__
int getV0FromRunLength(int numVertices, int av, int* adjacentVertices_runLength){
	//TODO: measure performance	
	//this: 
	//	pros, smaller memory, 
	//	cons, need this loop to determine v0! (do intelligent search instead)
	//alternatively: save v0 as a second value per index of flat_adjacentVertices
	//	pros, v0 is always known
	//	cons flat_adjacentVertices doubles in size
	int v0;
	for(int v = 0; v < numVertices; v++){
		if(av < adjacentVertices_runLength[v]){
			//printf("[%d, %d, %d, %d]:", blockIndex, local_threadIndex, global_threadIndex, av);
			v0 = v;
			break;
		}
	}
	return v0;
}

__device__
double cuda_l2norm_diff(int vi, int v0, double* flat_vertices){
	return sqrt((double) (flat_vertices[(vi*3)+0] - flat_vertices[(v0*3)+0])*(flat_vertices[(vi*3)+0] - flat_vertices[(v0*3)+0])
					   + (flat_vertices[(vi*3)+1] - flat_vertices[(v0*3)+1])*(flat_vertices[(vi*3)+1] - flat_vertices[(v0*3)+1])
					   + (flat_vertices[(vi*3)+2] - flat_vertices[(v0*3)+2])*(flat_vertices[(vi*3)+2] - flat_vertices[(v0*3)+2]));
}

__global__
void getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* flat_vertices, double* edgeLengths, double* minEdgeLength){
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x;
	int stride = blockDim.x * gridDim.x;
	// Use all availble threads to do all numVertices as v0
	for(int v0 = global_threadIndex; v0 < numVertices; v0 += stride){
		int av_begin = (v0 == 0 ? 0 : adjacentVertices_runLength[v0-1]);
		for(int av = av_begin; av < adjacentVertices_runLength[v0]; av++){
			if(minEdgeLength[v0] <= 0 || edgeLengths[av] <= minEdgeLength[v0]){
				minEdgeLength[v0] = edgeLengths[av];
			}
		}
		//printf("minEdgeLength[%d] %f\n", v0, minEdgeLength[v0]);
	}
}

__global__
void getFPrimes(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* featureVectors, double* minEdgeLength, double* flat_vertices, double* f_primes){
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	int stride = blockDim.x * gridDim.x; //32*3 = 96
	for(int av = global_threadIndex; av < numAdjacentVertices; av += stride){
		int vi = flat_adjacentVertices[av];
		int v0 = getV0FromRunLength(numVertices, av, adjacentVertices_runLength);
		f_primes[av] = featureVectors[v0] + minEdgeLength[v0] * (featureVectors[vi] - featureVectors[v0]) / cuda_l2norm_diff(vi, v0, flat_vertices);
		//printf("f_primes[%d]\t(v0 %d, vi %d)\t%g\n", av, v0, vi, f_primes[av]);
	}
}

__global__
void getCircleSectors(int numVertices, int* facesOfVertices_runLength, int* flat_facesOfVertices, int* flat_faces, double* edgeLengths){
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	int stride = blockDim.x * gridDim.x; //32*3 = 96

	double accuFuncVals = 0.0;
	double accuArea = 0.0;
	
	// Use all availble threads to do all numVertices as v0
	for(int v0 = global_threadIndex; v0 < numVertices; v0 += stride){
		int fi_begin = (v0 == 0 ? 0 : facesOfVertices_runLength[v0-1]);
		for(int fi = fi_begin; fi < facesOfVertices_runLength[v0]; fi++){
			//currFace->getFuncVal1RingSector( this, rMinDist, currArea, currFuncVal ); //ORS.307
				//get1RingSectorConst();
				int vi, vip1;
				getViAndVip1FromV0andFi(v0, flat_facesOfVertices[fi], flat_faces, vi, vip1);
				printf("[%d]\t[%d]\t%d\t%d\n", v0, flat_facesOfVertices[fi], vi, vip1);
				
				double alpha = edgeLengths[]
				/*
					// Fetch angle
					double alpha = getAngleAtVertex( rVert1RingCenter );
						double alpha;
						double lengthEdgeA = getLengthBC();
						double lengthEdgeB = getLengthCA();
						double lengthEdgeC = getLengthAB();
						alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2*lengthEdgeB*lengthEdgeC ) );
	
					// Area - https://en.wikipedia.org/wiki/Circular_sector#Area
					r1RingSecPre.mSectorArea = rNormDist * rNormDist * alpha / 2.0; // As alpha is already in radiant.

					// Truncated prism - function value equals the height
					if( !getOposingVertices( rVert1RingCenter, r1RingSecPre.mVertOppA, r1RingSecPre.mVertOppB ) ) {
					cerr << "[Face::" << __FUNCTION__ << "] ERROR: Finding opposing vertices!" << endl;
					return( false );
					}

					// Function values interpolated f'_i and f'_{i+1}
					// Compute the third angle using alpha/2.0 and 90°:
					double beta = ( M_PI - alpha ) / 2.0;
					// Law of sines
					double diameterCircum = rNormDist / sin( beta ); // Constant ratio equal longest edge
					// Distances for interpolation
					double lenCenterToA = distance( rVert1RingCenter, r1RingSecPre.mVertOppA );
					double lenCenterToB = distance( rVert1RingCenter, r1RingSecPre.mVertOppB );
					r1RingSecPre.mRatioCA = diameterCircum / lenCenterToA;
					r1RingSecPre.mRatioCB = diameterCircum / lenCenterToB;
					// Circle segment, center of gravity - https://de.wikipedia.org/wiki/Geometrischer_Schwerpunkt#Kreisausschnitt
					r1RingSecPre.mCenterOfGravityDist = ( 2.0 * sin( alpha ) ) / ( 3.0 * alpha );

				
				// Fetch function values
				double funcValCenter;
				double funcValA;
				double funcValB;
				rVert1RingCenter->getFuncValue( &funcValCenter );
				oneRingSecPre.mVertOppA->getFuncValue( &funcValA );
				oneRingSecPre.mVertOppB->getFuncValue( &funcValB );

				// Interpolate
				double funcValInterpolA = funcValCenter*(1.0-oneRingSecPre.mRatioCA) + funcValA*oneRingSecPre.mRatioCA;
				double funcValInterpolB = funcValCenter*(1.0-oneRingSecPre.mRatioCB) + funcValB*oneRingSecPre.mRatioCB;

				// Compute average function value at the center of gravity of the circle sector
				rSectorFuncVal = funcValCenter*( 1.0 - oneRingSecPre.mCenterOfGravityDist ) +
								 ( funcValInterpolA + funcValInterpolB ) * oneRingSecPre.mCenterOfGravityDist / 2.0;
				// Pass thru
				rSectorArea = oneRingSecPre.mSectorArea;
				return( true );
				
			double currFuncVal = 1.2;
			double currArea = 2;
			accuFuncVals += currFuncVal * currArea;
			accuArea += currArea;*/
		}
		
		//circleSectors[v0] = accuFuncVals / accuArea;
		//printf("minEdgeLength[%d] %f\n", v0, minEdgeLength[v0]);
	}
}

__device__
void getViAndVip1FromV0andFi(int v0, int fi, int* flat_faces, int& vi, int& vip1){
	//printf("flat_faces[%d*3+{0,1,2}] {%d, %d, %d}\n", fi, flat_faces[(fi*3)+0], flat_faces[(fi*3)+1], flat_faces[(fi*3)+2]);
	bool isViAssigned = false;
	for(int i = 0; i < 3; i++){ // for each vertex in this face (a, b, c)
		int v = flat_faces[fi*3+i];
		if(v != v0){ // exclude v0
			if(isViAssigned){
				vip1 = v; // assign the other corner to vip1
			}else{
				vi = v; // assign the first corner to vi
				isViAssigned = true;
			}
		}
	}
}


