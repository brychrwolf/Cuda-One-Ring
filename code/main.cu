#include <stdio.h>

#include <array>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <iterator>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// to engage GPUs when installed in hybrid system, run as 
// optirun ./main

template<typename T>
std::vector<T> split(std::string line){
	std::istringstream iss(line);
	std::vector<T> results(std::istream_iterator<T>{iss},
						   std::istream_iterator<T>());
	return results;
}
template std::vector<std::string> split<std::string>(std::string);
template std::vector<int> split<int>(std::string);
template std::vector<float> split<float>(std::string);
template std::vector<double> split<double>(std::string);

void printCUDAProps(int devCount);
void loadMesh_ply(std::string fileName, int& numVertices, double** vertices, double** featureVectors, int& numFaces, int** faces);
void printMesh(int numVertices, double* vertices, double* featureVectors, int numFaces, int* faces);

__global__ void buildLookupTables(int numFaces, int* faces, int* facesOfVertices, int* adjacentVertices);
__global__ void getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths);
__device__ int getV0FromRunLength(int numVertices, int av, int* adjacentVertices_runLength);
__device__ double cuda_l2norm_diff(int vi, int v0, double* vertices);
__global__ void getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength);
__global__ void getFPrimes(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* featureVectors, double* minEdgeLength, double* vertices, double* f_primes);
__global__ void getCircleSectors(
	int numVertices, 
	int* adjacentVertices_runLength,
	int* facesOfVertices_runLength, 
	int* flat_facesOfVertices, 
	int* flat_adjacentVertices,
	int* faces, 
	double* minEdgeLength, 
	double* featureVectors, 
	double* edgeLengths,
	double* circleSectors
);
__device__ void getViAndVip1FromV0andFi(int v0, int fi, int* faces, int& vi, int& vip1);
__device__ double getEdgeLengthOfV0AndVi(int v0, int vi, int* adjacentVertices_runLength, int* flat_adjacentVertices, double* edgeLengths);

int main(){
	/*************************************************************************/
	std::cout << std::endl << "****** Initializing CUDA..." << std::endl;
	/*************************************************************************/
	int devCount;
	cudaGetDeviceCount(&devCount);
	printf("CUDA Device Query...\n");
	if(devCount <= 0)
		std::cout << "No CUDA devices found." << std::endl;
	else
		printCUDAProps(devCount);
	int blockSize;
	int numBlocks;
	/*************************************************************************/
	std::cout << "****** CUDA Initialized." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Loading Mesh..." << std::endl;
	/*************************************************************************/
	int numVertices;
	int numFaces;
	double* vertices;
	double* featureVectors;
	int* faces;

	loadMesh_ply("../example_meshes/h.ply", numVertices, &vertices, &featureVectors, numFaces, &faces);
	//printMesh(numVertices, vertices, featureVectors, numFaces, faces);
	/*************************************************************************/
	std::cout << "****** Finished Loading." << std::endl;
	/*************************************************************************/


	
	/*************************************************************************/
	std::cout << std::endl << "****** Begin Building Tables..." << std::endl;
	/*************************************************************************/
	std::cout << "Building table of faces by vertex, " << std::endl;
	std::cout << "and table of adjacent vertices by vertex..." << std::endl;
	std::set<int> facesOfVertices[numVertices] = {};
	std::set<int> adjacentVertices[numVertices] = {};
	
	//int numCombos = numFaces * numVertices;
	//int blockSize = 256;
	//int numBlocks = (numCombos + blockSize - 1) / blockSize;
	//buildLookupTables<<<numBlocks, blockSize>>>(numFaces, faces, facesOfVertices, adjacentVertices);

	std::cout << "Iterating over each face as f..." << std::endl;
	//TODO: Determine if this way is optimal:
	//	edges saved twice, once in each direction, but enables use of runLength array...	

	for(int f = 0; f < numFaces; f++){
		for(int i = 0; i < 3; i++){ //TODO: relies on there always being 3 vertices to a face
			int a = f*3+(i+0)%3;
			int b = f*3+(i+1)%3;
			int c = f*3+(i+2)%3;
			int v = faces[a];
			facesOfVertices[v].insert(f);
			adjacentVertices[v].insert(faces[b]);
			adjacentVertices[v].insert(faces[c]);
		}
	}
	
	/*// Print facesOfVertices
	for(int v = 0; v < numVertices; v++){
		std::cerr << "facesOfVertices[" << v << "] ";
		for(int elem : facesOfVertices[v])
			std::cerr << elem << " ";
		std::cerr << std::endl;
	}
	// Print adjacentVertices
	for(int v = 0; v < numVertices; v++){
		std::cerr << "adjacentVertices[" << v << "] ";
		for(int elem : adjacentVertices[v])
			std::cerr << elem << " ";
		std::cerr << std::endl;
	}*/
	
	// Determine runlengths of adjacentVertices and facesofVertices
	int* adjacentVertices_runLength;
	int* facesOfVertices_runLength;
	cudaMallocManaged(&adjacentVertices_runLength, numVertices*sizeof(int));
	cudaMallocManaged(&facesOfVertices_runLength,  numVertices*sizeof(int));
	adjacentVertices_runLength[0] = adjacentVertices[0].size();
	facesOfVertices_runLength[0]  = facesOfVertices[0].size();
	std::cout << "Iterating over each vertex as v0..." << std::endl;
	for(int v0 = 0+1; v0 < numVertices; v0++){
		adjacentVertices_runLength[v0] = adjacentVertices_runLength[v0-1] + adjacentVertices[v0].size();
		facesOfVertices_runLength[v0]  = facesOfVertices_runLength[v0-1]  + facesOfVertices[v0].size();
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
	getEdgeLengths<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, vertices, edgeLengths);
	cudaDeviceSynchronize();	//wait for GPU to finish before accessing on host
	/*************************************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/*************************************************************************/
	std::cout << "Calculating minimum edge length among adjacent vertices..." << std::endl;
	double* minEdgeLength;
	cudaMallocManaged(&minEdgeLength, numVertices*sizeof(double));
	blockSize = 8;
	numBlocks = max(1, numVertices / blockSize);
	std::cout << "getMinEdgeLength<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	getMinEdgeLength<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, adjacentVertices_runLength, vertices, edgeLengths, minEdgeLength);
	cudaDeviceSynchronize();

	std::cout << std::endl << "Calculating f', weighted mean f0 and fi by distance..." << std::endl;
	double* f_primes;
	cudaMallocManaged(&f_primes, numAdjacentVertices*sizeof(double));
	blockSize = 32;
	numBlocks = max(1, numAdjacentVertices / blockSize);
	std::cout << "getFPrimes<<<" << numBlocks << ", " << blockSize << ">>(" << numAdjacentVertices << ")" << std::endl;
	getFPrimes<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, featureVectors, minEdgeLength, vertices, f_primes);
	cudaDeviceSynchronize();
	
	std::cout << std::endl << "Calculating circle_sectors..." << std::endl;
	double* circleSectors;
	cudaMallocManaged(&circleSectors, numVertices*sizeof(double));
	blockSize = 8;
	numBlocks = max(1, numVertices / blockSize);
	std::cout << "getCircleSectors<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	getCircleSectors<<<numBlocks, blockSize>>>(
		numVertices, 
		adjacentVertices_runLength, 
		facesOfVertices_runLength, 
		flat_facesOfVertices, 
		flat_adjacentVertices, 
		faces, 
		minEdgeLength, 
		featureVectors, 
		edgeLengths, 
		circleSectors
	);
	cudaDeviceSynchronize();
	/*************************************************************************/
	std::cout << "****** Finished Calculating." << std::endl;
	/*************************************************************************/
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

void loadMesh_ply(std::string fileName, int& numVertices, double** vertices, double** featureVectors, int& numFaces, int** faces){
	bool inHeaderSection = true;
	int faceSectionBegin;
	int vi = 0;
	int fi = 0;
	
	int v_idx = 0;
	int x_idx;
	int y_idx;
	int z_idx;

	std::ifstream infile(fileName);

	// read every line in the file
	std::string line;
	int lineNumber = 0;
	while(std::getline(infile, line)){
		// 3 sections: header, vertices, faces
		if(inHeaderSection){
			// parse for numVertices and numFaces
			if(line.substr(0, 7) == "element"){
				if(line.substr(8, 6) == "vertex"){
					std::vector<std::string> words = split<std::string>(line);
					std::istringstream convert(words[2]);
					convert >> numVertices;
				}else if(line.substr(8, 4) == "face"){
					std::vector<std::string> words = split<std::string>(line);
					std::istringstream convert(words[2]);
					convert >> numFaces;
				}
			// parse for coord indexes
			}else if(line.substr(0, 8) == "property"){
				std::vector<std::string> words = split<std::string>(line);
				if(words[2] == "x")
					x_idx = v_idx;
				else if(words[2] == "y")
					y_idx = v_idx;
				else if(words[2] == "z")
					z_idx = v_idx;
				v_idx++;
			}else if(line.substr(0, 10) == "end_header"){
				inHeaderSection = false;
				faceSectionBegin = lineNumber + 1 + numVertices;
				//(*vertices) = (double*) malloc(3 * numVertices * sizeof(double));
				//(*faces) = (int*) malloc(3 * numFaces * sizeof(int));
				cudaMallocManaged(&(*vertices), 3 * numVertices * sizeof(double));
				cudaMallocManaged(&(*featureVectors), numVertices * sizeof(double));
				cudaMallocManaged(&(*faces), 3 * numFaces * sizeof(int));
			}
		}else if(lineNumber < faceSectionBegin){
			std::vector<double> coords = split<double>(line);
			(*vertices)[vi*3 + 0] = coords[x_idx];
			(*vertices)[vi*3 + 1] = coords[y_idx];
			(*vertices)[vi*3 + 2] = coords[z_idx];
			//TODO: Are feature vectors stored in PLY file?
			(*featureVectors)[vi] = 1;
			vi++;
		}else{
			std::vector<int> coords = split<int>(line);
			(*faces)[fi*3 + 0] = coords[1]; //coords[0] is list size
			(*faces)[fi*3 + 1] = coords[2];
			(*faces)[fi*3 + 2] = coords[3];
			fi++;
		}
		lineNumber++;
	}
}

void printMesh(
	int numVertices, 
	double* vertices, 
	double* featureVectors, 
	int numFaces, 
	int* faces
){
	for(int v = 0; v < numVertices; v++){
		std::cout << "vertices[" << v << "] = ";
		for(int i=0; i < 3; i++){
			if(i > 0)
				std::cout << ", ";
			std::cout << vertices[v*3+i];
		}
		std::cout << " featureVector = " << featureVectors[v] << std::endl;
	}
	for(int f = 0; f < numFaces; f++)
		std::cout << f << " = {" << faces[f*3+0] << ", " << faces[f*3+1] << ", " << faces[f*3+2] << "}" <<std::endl;
}


__global__
void buildLookupTables(int numFaces, int* faces, int* facesOfVertices, int* adjacentVertices){
	//int index = blockIdx.x * blockDim.x + threadIdx.x;
	//int stride = blockDim.x * gridDim.x;

	//int v = index / numFaces;
	//int f = index % numFaces;
}

__global__
void getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths){
	//TODO Optimization analysis: storage vs speed
	//this:
	//	flat_adjacentVertices = 6nV
	//	adjacentVertices_runLength = 1nV
	//	index search requires averagePairCount per Vertex (6nV)
	//fully indexed:
	//	flat_adjacentVertices = 3*6nV (can be halved if redundant AVs are not stored)
	//	no runLength required
	//	no index search time
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	int stride = blockDim.x * gridDim.x; //32*3 = 96

	// Use all availble threads to do all numAdjacentVertices
	for(int av = global_threadIndex; av < numAdjacentVertices; av += stride){
		int vi = flat_adjacentVertices[av];
		int v0 = getV0FromRunLength(numVertices, av, adjacentVertices_runLength);
		edgeLengths[av] = cuda_l2norm_diff(vi, v0, vertices);
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
double cuda_l2norm_diff(int vi, int v0, double* vertices){
	return sqrt((double) (vertices[(vi*3)+0] - vertices[(v0*3)+0])*(vertices[(vi*3)+0] - vertices[(v0*3)+0])
					   + (vertices[(vi*3)+1] - vertices[(v0*3)+1])*(vertices[(vi*3)+1] - vertices[(v0*3)+1])
					   + (vertices[(vi*3)+2] - vertices[(v0*3)+2])*(vertices[(vi*3)+2] - vertices[(v0*3)+2]));
}

__global__
void getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength){
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
void getFPrimes(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* featureVectors, double* minEdgeLength, double* vertices, double* f_primes){
	int global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	int stride = blockDim.x * gridDim.x; //32*3 = 96
	for(int av = global_threadIndex; av < numAdjacentVertices; av += stride){
		int vi = flat_adjacentVertices[av];
		int v0 = getV0FromRunLength(numVertices, av, adjacentVertices_runLength);
		f_primes[av] = featureVectors[v0] + minEdgeLength[v0] * (featureVectors[vi] - featureVectors[v0]) / cuda_l2norm_diff(vi, v0, vertices);
		//printf("f_primes[%d]\t(v0 %d, vi %d)\t%g\n", av, v0, vi, f_primes[av]);
	}
}

__global__
void getCircleSectors(
	int numVertices, 
	int* adjacentVertices_runLength,
	int* facesOfVertices_runLength, 
	int* flat_facesOfVertices, 
	int* flat_adjacentVertices,
	int* faces, 
	double* minEdgeLength, 
	double* featureVectors, 
	double* edgeLengths,
	double* circleSectors
){
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
				getViAndVip1FromV0andFi(v0, flat_facesOfVertices[fi], faces, vi, vip1);
				//printf("[%d]\t[%d]\t%d\t%d\n", v0, flat_facesOfVertices[fi], vi, vip1);

				//TODO: Ensure edges A, B, C are correct with v0, vi, vip1; also regarding funcVals later
				//ORS.456
				double lengthEdgeA = getEdgeLengthOfV0AndVi(vi, vip1, adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double lengthEdgeB = getEdgeLengthOfV0AndVi(v0, vip1, adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double lengthEdgeC = getEdgeLengthOfV0AndVi(v0, vi,   adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2*lengthEdgeB*lengthEdgeC ) );

				double rNormDist = minEdgeLength[v0];
				double lenCenterToA = lengthEdgeC;
				double lenCenterToB = lengthEdgeB;
			
				//ORS.403 Area - https://en.wikipedia.org/wiki/Circular_sector#Area
				//*changed from m to r to skip "passthrough" see ORS.372
				double rSectorArea = rNormDist * rNormDist * alpha / 2.0; // As alpha is already in radiant.

				//ORS.412 Function values interpolated f'_i and f'_{i+1}
				// Compute the third angle using alpha/2.0 and 90°:
				double beta = ( M_PI - alpha ) / 2.0;
				// Law of sines
				double diameterCircum = rNormDist / sin( beta ); // Constant ratio equal longest edge

				//ORS.420 Distances for interpolation
				double mRatioCA = diameterCircum / lenCenterToA;
				double mRatioCB = diameterCircum / lenCenterToB;
				// Circle segment, center of gravity - https://de.wikipedia.org/wiki/Geometrischer_Schwerpunkt#Kreisausschnitt
				double mCenterOfGravityDist = ( 2.0 * sin( alpha ) ) / ( 3.0 * alpha );

				//ORS.357 Fetch function values
				double funcValCenter = featureVectors[v0];
				double funcValA = featureVectors[vi];
				double funcValB = featureVectors[vip1];

				//ORS.365 Interpolate
				double funcValInterpolA = funcValCenter*(1.0-mRatioCA) + funcValA*mRatioCA;
				double funcValInterpolB = funcValCenter*(1.0-mRatioCB) + funcValB*mRatioCB;

				//ORS.369 Compute average function value at the center of gravity of the circle sector
				double rSectorFuncVal = funcValCenter*( 1.0 - mCenterOfGravityDist ) +
								 ( funcValInterpolA + funcValInterpolB ) * mCenterOfGravityDist / 2.0;

			double currFuncVal = rSectorFuncVal;
			double currArea = rSectorArea;
			
			//ORS.309
			accuFuncVals += currFuncVal * currArea;
			accuArea += currArea;
		}

		circleSectors[v0] = accuFuncVals / accuArea;
		printf("circleSectors[%d] %f\n", v0, circleSectors[v0]);
	}
}

__device__
void getViAndVip1FromV0andFi(int v0, int fi, int* faces, int& vi, int& vip1){
	//printf("faces[%d*3+{0,1,2}] {%d, %d, %d}\n", fi, faces[(fi*3)+0], faces[(fi*3)+1], faces[(fi*3)+2]);
	bool isViAssigned = false;
	for(int i = 0; i < 3; i++){ // for each vertex in this face (a, b, c)
		int v = faces[fi*3+i];
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


__device__
double getEdgeLengthOfV0AndVi(int v0, int vi, int* adjacentVertices_runLength, int* flat_adjacentVertices, double* edgeLengths){
	//TODO: Error handling?
	int av_begin = (v0 == 0 ? 0 : adjacentVertices_runLength[v0-1]);
	double edgeLength;
	for(int av = av_begin; av < adjacentVertices_runLength[v0]; av++){
		if(flat_adjacentVertices[av] == vi){
			edgeLength = edgeLengths[av];
			break;
		}
	}
	return edgeLength;
}


