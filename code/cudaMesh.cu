#include <iostream>
#include <fstream>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "cudaMesh.cuh"
#include "cudaAccess.cuh"

//TODO: Only loads PLY files, should support other file types!

CudaMesh::CudaMesh(){
	//TODO: implement
}

CudaMesh::CudaMesh(CudaAccess* acc){
	ca = acc;
}

CudaMesh::~CudaMesh(){
}



/* Getters and Setters */
int CudaMesh::getNumVertices(){
	return numVertices;
}

int CudaMesh::getNumFaces(){
	return numFaces;
}

double* CudaMesh::getVertices(){
	return vertices;
}

double* CudaMesh::getFeatureVectors(){
	return featureVectors;
}

int* CudaMesh::getFaces(){
	return faces;
}

std::vector<std::set<int>> CudaMesh::getAdjacentVertices(){
	return adjacentVertices;
}

std::vector<std::set<int>> CudaMesh::getFacesOfVertices(){
	return facesOfVertices;
}

int* CudaMesh::getAdjacentVertices_runLength(){
	return adjacentVertices_runLength;
}

int* CudaMesh::getFacesOfVertices_runLength(){
	return facesOfVertices_runLength;
}

int CudaMesh::getNumAdjacentVertices(){
	return numAdjacentVertices;
}

int CudaMesh::getNumFacesOfVertices(){
	return numFacesOfVertices;
}

int* CudaMesh::getFlat_adjacentVertices(){
	return flat_adjacentVertices;
}

int* CudaMesh::getFlat_facesOfVertices(){
	return flat_facesOfVertices;
}

double* CudaMesh::getEdgeLengths(){
	return edgeLengths;
}

double* CudaMesh::getMinEdgeLength(){
	return minEdgeLength;
}



void CudaMesh::setNumVertices(int upd){
	numVertices = upd;
}

void CudaMesh::setNumFaces(int upd){
	numFaces = upd;
}

void CudaMesh::setVertices(double* upd){
	vertices = upd;
}

void CudaMesh::setFeatureVectors(double* upd){
	featureVectors = upd;
}

void CudaMesh::setFaces(int* upd){
	faces = upd;
}

void CudaMesh::setAdjacentVertices(std::vector<std::set<int>> upd){
	adjacentVertices = upd;
}

void CudaMesh::setFacesOfVertices(std::vector<std::set<int>> upd){
	facesOfVertices = upd;
}

void CudaMesh::setAdjacentVertices_runLength(int* upd){
	adjacentVertices_runLength = upd;
}

void CudaMesh::setFacesOfVertices_runLength(int* upd){
	facesOfVertices_runLength = upd;
}

void CudaMesh::setNumAdjacentVertices(int upd){
	numAdjacentVertices = upd;
}

void CudaMesh::setNumFacesOfVertices(int upd){
	numFacesOfVertices = upd;
}

void CudaMesh::setFlat_adjacentVertices(int* upd){
	flat_adjacentVertices = upd;
}

void CudaMesh::setFlat_facesOfVertices(int* upd){
	flat_facesOfVertices = upd;
}

void CudaMesh::setEdgeLengths(double* upd){
	edgeLengths = upd;
}

void CudaMesh::setMinEdgeLength(double* upd){
	minEdgeLength = upd;
}



/* IO */
void CudaMesh::loadPLY(std::string fileName){
	bool inHeaderSection = true;
	int faceSectionBegin;
	int vi = 0;
	int fi = 0;
	
	int v_idx = 0;
	int x_idx;
	int y_idx;
	int z_idx;

	std::ifstream infile(fileName);
	
	// for Random Feature Vectors
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);

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
				cudaMallocManaged(&vertices, 3 * numVertices * sizeof(double));
				cudaMallocManaged(&featureVectors, numVertices * sizeof(double));
				cudaMallocManaged(&faces, 3 * numFaces * sizeof(int));
			}
		}else if(lineNumber < faceSectionBegin){
			std::vector<double> coords = split<double>(line);
			vertices[vi*3 + 0] = coords[x_idx];
			vertices[vi*3 + 1] = coords[y_idx];
			vertices[vi*3 + 2] = coords[z_idx];
			//TODO: Are feature vectors stored in PLY file? currently set to 1 or random
			featureVectors[vi] = dis(gen);//1;
			vi++;
		}else{
			std::vector<int> coords = split<int>(line);
			faces[fi*3 + 0] = coords[1]; //coords[0] is list size
			faces[fi*3 + 1] = coords[2];
			faces[fi*3 + 2] = coords[3];
			fi++;
		}
		lineNumber++;
	}
}



void CudaMesh::printMesh(){
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

void CudaMesh::printAdjacentVertices(){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "adjacentVertices[" << i << "] ";
		for(int elem : adjacentVertices[i])
			std::cerr << elem << " ";
		std::cerr << std::endl;
	}
}

void CudaMesh::printFacesOfVertices(){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "facesOfVertices[" << i << "] ";
		for(int elem : facesOfVertices[i])
			std::cerr << elem << " ";
		std::cerr << std::endl;
	}
}

void CudaMesh::printAdjacentVertices_RunLength(){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "adjacentVertices_runLength[" << i << "] " << adjacentVertices_runLength[i] << std::endl;
	}
}

void CudaMesh::printFacesOfVertices_RunLength(){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "facesOfVertices_runLength[" << i << "] " << facesOfVertices_runLength[i] << std::endl;
	}
}

void CudaMesh::printFlat_AdjacentVertices(){
	std::cerr << std::endl;
	for(int i = 0; i < numAdjacentVertices; i++){
		std::cerr << "flat_adjacentVertices[" << i << "] " << flat_adjacentVertices[i] << std::endl;
	}
}

void CudaMesh::printFlat_FacesOfVertices(){
	std::cerr << std::endl;
	for(int i = 0; i < numFacesOfVertices; i++){
		std::cerr << "flat_facesOfVertices[" << i << "] " << flat_facesOfVertices[i] << std::endl;
	}
}

void CudaMesh::printEdgeLengths(){
	std::cerr << std::endl;
	for(int i = 0; i < numAdjacentVertices; i++){
		std::cerr << "edgeLengths[" << i << "] " << edgeLengths[i] << std::endl;
	}
}

void CudaMesh::printMinEdgeLength(){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "minEdgeLength[" << i << "] " << minEdgeLength[i] << std::endl;
	}
}



/* Build Tables */
void CudaMesh::buildSets(){
	std::vector<std::set<int>>(numVertices).swap(adjacentVertices);
	std::vector<std::set<int>>(numVertices).swap(facesOfVertices);

	//TODO: Determine if this way is optimal:
	//	edges saved twice, once in each direction, but enables use of runLength array...
	for(int f = 0; f < numFaces; f++){
		for(int i = 0; i < 3; i++){ //TODO: relies on there always being 3 vertices to a face
			int a = f*3+(i+0)%3;
			int b = f*3+(i+1)%3;
			int c = f*3+(i+2)%3;
			int v = faces[a];
			adjacentVertices[v].insert(faces[b]);
			adjacentVertices[v].insert(faces[c]);
			facesOfVertices[v].insert(f);
		}
	}
}

void CudaMesh::determineRunLengths(){
	cudaMallocManaged(&adjacentVertices_runLength, numVertices*sizeof(int));
	cudaMallocManaged(&facesOfVertices_runLength,  numVertices*sizeof(int));
	
	std::cout << "Iterating over each vertex as v0..." << std::endl;
	adjacentVertices_runLength[0] = adjacentVertices[0].size();
	facesOfVertices_runLength[0]  = facesOfVertices[0].size();
	for(int v0 = 0+1; v0 < numVertices; v0++){
		adjacentVertices_runLength[v0] = adjacentVertices_runLength[v0-1] + adjacentVertices[v0].size();
		facesOfVertices_runLength[v0]  = facesOfVertices_runLength[v0-1]  + facesOfVertices[v0].size();
	}
	
	numAdjacentVertices = adjacentVertices_runLength[numVertices-1];
	numFacesOfVertices  = facesOfVertices_runLength[numVertices-1];
}

void CudaMesh::flattenSets(){
	cudaMallocManaged(&flat_adjacentVertices, numAdjacentVertices*sizeof(int));
	cudaMallocManaged(&flat_facesOfVertices, numFacesOfVertices*sizeof(int));

	int r = 0;
	int s = 0;
	for(int v0 = 0; v0 < numVertices; v0++){
		for(std::set<int>::iterator vi_iter = adjacentVertices[v0].begin(); vi_iter != adjacentVertices[v0].end(); vi_iter++){
			int vi = *vi_iter;
			flat_adjacentVertices[r] = vi;
			r++;
		}
		for(std::set<int>::iterator vi_iter = facesOfVertices[v0].begin(); vi_iter != facesOfVertices[v0].end(); vi_iter++){
			int vi = *vi_iter;
			flat_facesOfVertices[s] = vi;
			s++;
		}
	}
}



/* Pre-Calculation */
void CudaMesh::preCalculateEdgeLengths(){
	cudaMallocManaged(&edgeLengths, numAdjacentVertices*sizeof(double));
	int blockSize = (*ca).getIdealBlockSizeForProblemOfSize(numAdjacentVertices);
	int numBlocks = max(1, numAdjacentVertices / blockSize);
	std::cout << "getEdgeLengths<<<" << numBlocks << ", " << blockSize <<">>(" << numAdjacentVertices << ")" << std::endl;
	kernel_getEdgeLengths<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, vertices, edgeLengths);
	cudaDeviceSynchronize();	//wait for GPU to finish before accessing on host
}

__global__
void kernel_getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths){
	//TODO Optimization analysis: storage vs speed
	//this:
	//	flat_adjacentVertices = 6nV (average 6 pairs per vertex)
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
	// Too slow
	return sqrt((double) (vertices[(vi*3)+0] - vertices[(v0*3)+0])*(vertices[(vi*3)+0] - vertices[(v0*3)+0])
					   + (vertices[(vi*3)+1] - vertices[(v0*3)+1])*(vertices[(vi*3)+1] - vertices[(v0*3)+1])
					   + (vertices[(vi*3)+2] - vertices[(v0*3)+2])*(vertices[(vi*3)+2] - vertices[(v0*3)+2]));
	/* Even slower...!?
	int vi30 = (vi * 3);
	int vi31 = (vi * 3) + 1;
	int vi32 = (vi * 3) + 2;
	int v030 = (v0 * 3);
	int v031 = (v0 * 3) + 1;
	int v032 = (v0 * 3) + 2;
	return sqrt((double) (vertices[vi30] - vertices[v030]) * (vertices[vi30] - vertices[v030])
					   + (vertices[vi31] - vertices[v031]) * (vertices[vi31] - vertices[v031])
					   + (vertices[vi32] - vertices[v032]) * (vertices[vi32] - vertices[v032]));*/
}

void CudaMesh::preCalculateMinEdgeLength(){
	cudaMallocManaged(&minEdgeLength, numVertices*sizeof(double));
	int blockSize = (*ca).getIdealBlockSizeForProblemOfSize(numVertices);
	int numBlocks = max(1, numVertices / blockSize);
	std::cout << "getMinEdgeLength<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	kernel_getMinEdgeLength<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, adjacentVertices_runLength, vertices, edgeLengths, minEdgeLength);
	cudaDeviceSynchronize();
}

__global__
void kernel_getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength){
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

