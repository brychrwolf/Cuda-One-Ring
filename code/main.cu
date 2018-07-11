#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>
#include <array>
#include <vector>
#include <map>

int P0_BEGIN; // to modify the loop over which 
int P0_END;   // vertices are computed 

// to engage GPUs when installed in hybrid system, run as 
// optirun ./main

typedef std::array<double, 3> vertex;
typedef std::array<int, 3> face;

vertex scale(vertex v, double scalar);
vertex combine(vertex v1, vertex v2);
double l2norm(const vertex pi);
double l2norm_diff(const vertex pi, const vertex p0);
void printCUDAProps(int devCount);
void loadMesh_syntheticH(vertex vertices[], double featureVectors[], face faces[]);
void flattenMesh(int numVertices, vertex vertices[], double flat_vertices[], double featureVectors[], int numFaces, face faces[], int flat_faces[]);
void printMesh(int numVertices, vertex vertices[], double featureVectors[], int numFaces, face faces[]);

__global__
void buildLookupTables(int numFaces, face faces[], std::set<int> facesOfVertices[], std::set<int> adjacentVertices[]){
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int stride = blockDim.x * gridDim.x;

	int v = index / numFaces;
	int f = index % numFaces;
	
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
__global__
void getMinEdgeLength(int numVertices, vertex cuda_vertices[], int numFaces, face cuda_faces[], int cuda_adjacentVertices[], double cuda_minEdgeLength[]){
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int stride = blockDim.x * gridDim.x;


	/*for(pi = 0; pi_iter < cuda_adjacentVertices[p0].size(); pi++){
		int pi = *pi_iter;
		double norm_diff = l2norm_diff(vertices[pi], vertices[p0]); //TODO: used twice, for p0 and when p1 becomes p0. Would saving value make a big difference?
		if(norm_diff <= minEdgeLength[p0]){
			minEdgeLength[p0] = norm_diff;
			minEdgeLength_vertex = pi;
		}
		std::cout  << "p0 " << p0 << " pi " << pi << " norm_diff " << norm_diff << std::endl;
	}*/
}

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
	/******************************************************************/
	std::cout << "****** CUDA Initialized (if available)." << std::endl;
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
	int P0_BEGIN = 0;
	int P0_END = numVertices;
	
	std::cout << "Building table of faces by vertex, " << std::endl;
	std::cout << "and table of adjacent vertices by vertex..." << std::endl;
	std::set<int> facesOfVertices[numVertices] = {};
	std::set<int> adjacentVertices[numVertices] = {};
	
	int numCombos = numFaces * numVertices;
	int blockSize = 256;
	int numBlocks = (numCombos + blockSize - 1) / blockSize;
	buildLookupTables<<<numBlocks, blockSize>>>(numFaces, faces, facesOfVertices, adjacentVertices);
	

	int totalAdjacentVertices = 0;
	int numAdjacentVertices[numVertices] = {};
	for(int p0 = P0_BEGIN; p0 < P0_END; p0++){
		numAdjacentVertices[p0] = adjacentVertices[p0].size();
		totalAdjacentVertices += adjacentVertices[p0].size();
		std::cout << "numAdjacentVertices[" << p0 << "] " << numAdjacentVertices[p0] << std::endl;
	}
	std::cout << "totalAdjacentVertices " << totalAdjacentVertices << std::endl;
	/***********************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/***********************************************************/



	/******************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/******************************************************************/
	double minEdgeLength[numVertices];
	std::fill_n(minEdgeLength, numVertices, FLT_MAX); // initialize array to max double value
	std::array<std::map<int, double>, numVertices> f_primes; // function value at delta_min along pi
	std::array<std::map<int, double>, numVertices> f_triangles; // function value of triangles 
	std::array<std::map<int, double>, numVertices> a_triangles_pythag; // area of geodesic triangles to be used as weights
	std::array<std::map<int, double>, numVertices> a_triangles_coord; // area of geodesic triangles to be used as weights
	double wa_geoDisks[numVertices] = {}; // weighted area of triangles comprising total geodiseic disk
	std::array<std::map<int, double>, numVertices> circle_sectors; //! Computes a circle sector and a mean function value of the corresponding prism at the center of gravity.

	int *cuda_adjacentVertices;
	double *cuda_minEdgeLength;
	cudaMallocManaged(&cuda_adjacentVertices, totalAdjacentVertices*sizeof(int));
	cudaMallocManaged(&cuda_minEdgeLength, numVertices*sizeof(double));

	//int blockSize = 2;
	//int numBlocks = (numVertices + blockSize - 1) / blockSize;
	//getMinEdgeLength<<<numBlocks, blockSize>>>(numVertices, cuda_vertices, numFaces, cuda_faces, cuda_adjacentVertices, cuda_minEdgeLength);
	cudaDeviceSynchronize();	//wait for GPU to finish before accessing on host

	std::cout << "Iterating over each vertex as p0..." << std::endl;
	for(int p0 = P0_BEGIN; p0 < P0_END; p0++){



		/*std::cout << "Calculating minimum edge length among adjacent vertices..." << std::endl;
		int minEdgeLength_vertex = -1; // a minimum must exist, error if none is found
		std::cout << "Iterating over each adjacent_vertex as pi..." << std::endl;
		for(std::set<int>::iterator pi_iter = adjacentVertices[p0].begin(); pi_iter != adjacentVertices[p0].end(); pi_iter++){
			int pi = *pi_iter;
			double norm_diff = l2norm_diff(vertices[pi], vertices[p0]); //TODO: used twice, for p0 and when p1 becomes p0. Would saving value make a big difference?
			if(norm_diff <= minEdgeLength[p0]){
				minEdgeLength[p0] = norm_diff;
				minEdgeLength_vertex = pi;
			}
			std::cout  << "p0 " << p0 << " pi " << pi << " norm_diff " << norm_diff << std::endl;
		}
		std::cout << "minEdgeLength[" << p0 << "] " << minEdgeLength[p0] << " minEdgeLength_vertex " << minEdgeLength_vertex << std::endl;
		std::cout << "cuda_minEdgeLength[" << p0 << "] " << cuda_minEdgeLength[p0] << " minEdgeLength_vertex " << minEdgeLength_vertex << std::endl;*/


		/*std::cout << std::endl << "Calculating f', weighted mean f0 and fi by distance..." << std::endl;
		std::cout << "Iterating over each adjacent_vertex as pi..." << std::endl;		
		for(std::set<int>::iterator pi_iter = adjacentVertices[p0].begin(); pi_iter != adjacentVertices[p0].end(); pi_iter++){
			int pi = *pi_iter;
			double f_prime = featureVectors[p0] + minEdgeLength[p0] * (featureVectors[pi] - featureVectors[p0]) / l2norm_diff(vertices[pi], vertices[p0]);
			f_primes[p0].insert(std::pair<int, double>(pi, f_prime));
			std::cout << "f_primes[" << p0 << "][" << pi << "] " << f_primes[p0][pi] << std::endl;
		}


		
		std::cout << std::endl << "Calculating f_triangles, weighted mean (f0 + f'i + f'ip1)/3..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi;
			int pip1;
			bool isPiAsigned = false;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				if(v != p0){ // exclude p0
					if(!isPiAsigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAsigned = true;
					}
				}
			}					
			
			double f_triangle = (featureVectors[p0] + f_primes[p0][pi] + f_primes[p0][pip1]); //save the /3 for later like in paper
			f_triangles[p0].insert(std::pair<int, double>(ti, f_triangle));
			std::cout << "f_triangles[" << p0 << "][" << ti << "] " << f_triangles[p0][ti] << std::endl;
		}


		
		std::cout << std::endl << "Calculating a_triangles_pythag, area to be used as weights..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi, pip1;
			bool isPiAssigned = false;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				if(v != p0){ // exclude p0
					if(isPiAssigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAssigned = true;
					}
				}
			}

			vertex relative_pi = combine(vertices[pi],   scale(vertices[p0], -1));
			vertex unit_pi = scale(relative_pi, 1/l2norm(relative_pi));
			vertex scaled_pi = scale(unit_pi, minEdgeLength[p0]);
			vertex mel_pi = combine(scaled_pi, vertices[p0]);
			
			vertex relative_pip1 = combine(vertices[pip1],   scale(vertices[p0], -1));
			vertex unit_pip1 = scale(relative_pip1, 1/l2norm(relative_pip1));
			vertex scaled_pip1 = scale(unit_pip1, minEdgeLength[p0]);
			vertex mel_pip1 = combine(scaled_pip1, vertices[p0]);
			
			double base = l2norm_diff(mel_pip1, mel_pi);
			double height = sqrt(minEdgeLength[p0]*minEdgeLength[p0] - (base/2)*(base/2));
			double a_triangle = base * height / 2;

			// or like as paper
			//double a_triangle = base/4 * sqrt(4*minEdgeLength[p0]*minEdgeLength[p0] - base*base); // multiplying by 4 inside the sqrt countered by dividing by 2 outside

			a_triangles_pythag[p0].insert(std::pair<int, double>(ti, a_triangle));
			std::cout << "a_triangles_pythag[" << p0 << "][" << ti << "] " << a_triangles_pythag[p0][ti] << std::endl;
		}



		std::cout << std::endl << "Calculating a_geoDisks, weighted mean function value over total area of adjacent triangles..." << std::endl;
		double area = 0.0;
		double weighted_area = 0.0;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			area += a_triangles_pythag[p0][ti];
			double wa = a_triangles_pythag[p0][ti] * f_triangles[p0][ti];
			weighted_area += wa;
			std::cout << "weighted_area[" << p0 << "]" << "[" << ti << "] = " << wa << std::endl;
		}
		std::cout << "total area " << area << std::endl;
		std::cout << "total weighted_area " << weighted_area << std::endl;
		double wa_geoDisk = weighted_area / (3 * area); // /3 was carried over from from the f_triangles calculations
		wa_geoDisks[p0] = (wa_geoDisk);
		std::cout << "wa_geoDisks[" << p0 << "] " << wa_geoDisks[p0] << std::endl;
		
		

		std::cout << std::endl << "Calculating circle_sectors..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){		
			int ti = *ti_iter;
			
		}*/
	}
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

double l2norm(const vertex pi){
	return sqrt(pi[0]*pi[0]
			  + pi[1]*pi[1]
			  + pi[2]*pi[2]);
}

double l2norm_diff(const vertex pi, const vertex p0){
	return sqrt((pi[0] - p0[0])*(pi[0] - p0[0])
			  + (pi[1] - p0[1])*(pi[1] - p0[1])
			  + (pi[2] - p0[2])*(pi[2] - p0[2]));
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
	}
	for(int f = 0; f < numFaces; f++){
		flat_faces[(f*3)+0] = faces[f][0];
		flat_faces[(f*3)+1] = faces[f][1];
		flat_faces[(f*3)+2] = faces[f][2];
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

