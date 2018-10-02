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
	globalMinEdgeLength = -1;
}

CudaMesh::~CudaMesh(){
}



/* Getters and Setters */
unsigned long CudaMesh::getNumVertices(){
	return numVertices;
}

unsigned long CudaMesh::getNumFaces(){
	return numFaces;
}

double* CudaMesh::getVertices(){
	return vertices;
}

double* CudaMesh::getFunctionValues(){
	return functionValues;
}

unsigned long* CudaMesh::getFaces(){
	return faces;
}

std::vector<std::set<unsigned long>> CudaMesh::getAdjacentVertices(){
	return adjacentVertices;
}

std::vector<std::set<unsigned long>> CudaMesh::getFacesOfVertices(){
	return facesOfVertices;
}

unsigned long* CudaMesh::getAdjacentVertices_runLength(){
	return adjacentVertices_runLength;
}

unsigned long* CudaMesh::getFacesOfVertices_runLength(){
	return facesOfVertices_runLength;
}

unsigned long CudaMesh::getNumAdjacentVertices(){
	return numAdjacentVertices;
}

unsigned long CudaMesh::getNumFacesOfVertices(){
	return numFacesOfVertices;
}

unsigned long* CudaMesh::getFlat_adjacentVertices(){
	return flat_adjacentVertices;
}

unsigned long* CudaMesh::getFlat_facesOfVertices(){
	return flat_facesOfVertices;
}

double* CudaMesh::getEdgeLengths(){
	return edgeLengths;
}

double* CudaMesh::getMinEdgeLength(){
	return minEdgeLength;
}

double CudaMesh::getGlobalMinEdgeLength(){
	return globalMinEdgeLength;
}

double* CudaMesh::getOneRingMeanFunctionValues(){
	return oneRingMeanFunctionValues;
}



void CudaMesh::setNumVertices(unsigned long upd){
	numVertices = upd;
}

void CudaMesh::setNumFaces(unsigned long upd){
	numFaces = upd;
}

void CudaMesh::setVertices(double* upd){
	vertices = upd;
}

void CudaMesh::setFunctionValues(double* upd){
	functionValues = upd;
}

void CudaMesh::setFaces(unsigned long* upd){
	faces = upd;
}

void CudaMesh::setAdjacentVertices(std::vector<std::set<unsigned long>> upd){
	adjacentVertices = upd;
}

void CudaMesh::setFacesOfVertices(std::vector<std::set<unsigned long>> upd){
	facesOfVertices = upd;
}

void CudaMesh::setAdjacentVertices_runLength(unsigned long* upd){
	adjacentVertices_runLength = upd;
}

void CudaMesh::setFacesOfVertices_runLength(unsigned long* upd){
	facesOfVertices_runLength = upd;
}

void CudaMesh::setNumAdjacentVertices(unsigned long upd){
	numAdjacentVertices = upd;
}

void CudaMesh::setNumFacesOfVertices(unsigned long upd){
	numFacesOfVertices = upd;
}

void CudaMesh::setFlat_adjacentVertices(unsigned long* upd){
	flat_adjacentVertices = upd;
}

void CudaMesh::setFlat_facesOfVertices(unsigned long* upd){
	flat_facesOfVertices = upd;
}

void CudaMesh::setEdgeLengths(double* upd){
	edgeLengths = upd;
}

void CudaMesh::setMinEdgeLength(double* upd){
	minEdgeLength = upd;
}

void CudaMesh::setGlobalMinEdgeLength(double upd){
	globalMinEdgeLength = upd;
}

void CudaMesh::setOneRingMeanFunctionValues(double* upd){
	oneRingMeanFunctionValues = upd;
}



/* IO */
void CudaMesh::loadPLY(std::string fileName){
	bool inHeaderSection = true;
	unsigned long faceSectionBegin;
	unsigned long vi = 0;
	unsigned long fi = 0;
	
	unsigned long v_idx = 0;
	unsigned long x_idx;
	unsigned long y_idx;
	unsigned long z_idx;

	std::ifstream infile(fileName);
	
	// read every line in the file
	std::string line;
	unsigned long lineNumber = 0;
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
				cudaMallocManaged(&functionValues, numVertices * sizeof(double));
				cudaMallocManaged(&faces, 3 * numFaces * sizeof(unsigned long));
			}
		}else if(lineNumber < faceSectionBegin){
			std::vector<double> coords = split<double>(line);
			vertices[vi*3 + 0] = coords[x_idx];
			vertices[vi*3 + 1] = coords[y_idx];
			vertices[vi*3 + 2] = coords[z_idx];
			vi++;
		}else{
			std::vector<unsigned long> coords = split<unsigned long>(line);
			faces[fi*3 + 0] = coords[1]; //coords[0] is list size
			faces[fi*3 + 1] = coords[2];
			faces[fi*3 + 2] = coords[3];
			fi++;
		}
		lineNumber++;
	}
}

void CudaMesh::loadFunctionValues(std::string fileName){
	//TODO: Complain if file doesn't exists
	//TODO: Complain if file exists but not all functionValues get set
	if(fileName == ""){
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(-1.0, 1.0);
		for(unsigned long vi = 0; vi < numVertices; vi++)
			functionValues[vi] = dis(gen); //1;
		std::cout << "functionValues set to random" << std::endl;
		return;
	}

	std::ifstream infile(fileName);
	std::string line;
	unsigned long vi = 0;
	while(std::getline(infile, line)){
		if(line.substr(0, 1) == "#") continue;
		std::vector<double> idsAndValues = split<double>(line);
		functionValues[vi] = idsAndValues[1];
		//if(vi % 1000 == 0) std::cerr << "functionValues[" << vi << "] " << functionValues[vi] << std::endl;
		vi++;
	}
}

void CudaMesh::writeFunctionValues(std::string fileName){
	std::ofstream outfile(fileName);
	for(unsigned long vi = 0; vi < numVertices; vi++)
		outfile << vi << " " << oneRingMeanFunctionValues[vi] << std::endl;
	outfile.close();
}



void CudaMesh::printMesh(){
	for(unsigned long v = 0; v < numVertices; v++){
		std::cout << "vertices[" << v << "] = ";
		for(int i=0; i < 3; i++){
			if(i > 0)
				std::cout << ", ";
			std::cout << vertices[v*3+i];
		}
		std::cout << " functionValue = " << functionValues[v] << std::endl;
	}
	for(unsigned long f = 0; f < numFaces; f++)
		std::cout << f << " = {" << faces[f*3+0] << ", " << faces[f*3+1] << ", " << faces[f*3+2] << "}" <<std::endl;
}

void CudaMesh::printAdjacentVertices(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "adjacentVertices[" << i << "] ";
		for(int elem : adjacentVertices[i])
			std::cout << elem << " ";
		std::cout << std::endl;
	}
}

void CudaMesh::printFacesOfVertices(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "facesOfVertices[" << i << "] ";
		for(int elem : facesOfVertices[i])
			std::cout << elem << " ";
		std::cout << std::endl;
	}
}

void CudaMesh::printAdjacentVertices_RunLength(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "adjacentVertices_runLength[" << i << "] " << adjacentVertices_runLength[i] << std::endl;
	}
}

void CudaMesh::printFacesOfVertices_RunLength(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "facesOfVertices_runLength[" << i << "] " << facesOfVertices_runLength[i] << std::endl;
	}
}

void CudaMesh::printFlat_AdjacentVertices(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numAdjacentVertices; i++){
		std::cout << "flat_adjacentVertices[" << i << "] " << flat_adjacentVertices[i] << std::endl;
	}
}

void CudaMesh::printFlat_FacesOfVertices(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numFacesOfVertices; i++){
		std::cout << "flat_facesOfVertices[" << i << "] " << flat_facesOfVertices[i] << std::endl;
	}
}

void CudaMesh::printEdgeLengths(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numAdjacentVertices; i++){
		std::cout << "edgeLengths[" << i << "] " << edgeLengths[i] << std::endl;
	}
}

void CudaMesh::printMinEdgeLength(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "minEdgeLength[" << i << "] " << minEdgeLength[i] << std::endl;
	}
}

void CudaMesh::printOneRingMeanFunctionValues(){
	std::cout << std::endl;
	for(unsigned long i = 0; i < numVertices; i++){
		std::cout << "oneRingMeanFunctionValues[" << i << "] " << oneRingMeanFunctionValues[i] << std::endl;
	}
}



/* Build Tables */
void CudaMesh::buildSets(){
	std::vector<std::set<unsigned long>>(numVertices).swap(adjacentVertices);
	std::vector<std::set<unsigned long>>(numVertices).swap(facesOfVertices);

	//TODO: Determine if this way is optimal:
	//	edges saved twice, once in each direction, but enables use of runLength array...
	for(unsigned long f = 0; f < numFaces; f++){
		for(int i = 0; i < 3; i++){ //TODO: relies on there always being 3 vertices to a face
			unsigned long a = f*3+(i+0)%3;
			unsigned long b = f*3+(i+1)%3;
			unsigned long c = f*3+(i+2)%3;
			unsigned long v = faces[a];
			adjacentVertices[v].insert(faces[b]);
			adjacentVertices[v].insert(faces[c]);
			facesOfVertices[v].insert(f);
		}
	}
}

void CudaMesh::determineRunLengths(){
	cudaMallocManaged(&adjacentVertices_runLength, numVertices*sizeof(unsigned long));
	cudaMallocManaged(&facesOfVertices_runLength,  numVertices*sizeof(unsigned long));
	
	std::cout << "Iterating over each vertex as v0..." << std::endl;
	adjacentVertices_runLength[0] = adjacentVertices[0].size();
	facesOfVertices_runLength[0]  = facesOfVertices[0].size();
	for(unsigned long v0 = 0+1; v0 < numVertices; v0++){
		adjacentVertices_runLength[v0] = adjacentVertices_runLength[v0-1] + adjacentVertices[v0].size();
		facesOfVertices_runLength[v0]  = facesOfVertices_runLength[v0-1]  + facesOfVertices[v0].size();
	}
	
	numAdjacentVertices = adjacentVertices_runLength[numVertices-1];
	numFacesOfVertices  = facesOfVertices_runLength[numVertices-1];
}

void CudaMesh::flattenSets(){
	cudaMallocManaged(&flat_adjacentVertices, numAdjacentVertices*sizeof(unsigned long));
	cudaMallocManaged(&flat_facesOfVertices, numFacesOfVertices*sizeof(unsigned long));

	unsigned long vi_av = 0;
	unsigned long vi_fv = 0;
	for(unsigned long v0 = 0; v0 < numVertices; v0++){
		for(std::set<unsigned long>::iterator vi_iter = adjacentVertices[v0].begin(); vi_iter != adjacentVertices[v0].end(); vi_iter++){
			unsigned long vi = *vi_iter;
			flat_adjacentVertices[vi_av] = vi;
			vi_av++;
		}
		for(std::set<unsigned long>::iterator vi_iter = facesOfVertices[v0].begin(); vi_iter != facesOfVertices[v0].end(); vi_iter++){
			unsigned long vi = *vi_iter;
			flat_facesOfVertices[vi_fv] = vi;
			vi_fv++;
		}
	}
}

void CudaMesh::freeSets(){
	std::vector<std::set<unsigned long>>().swap(adjacentVertices);
	std::vector<std::set<unsigned long>>().swap(facesOfVertices);
}



/* Pre-Calculation */
void CudaMesh::preCalculateEdgeLengths(){
	cudaMallocManaged(&edgeLengths, numAdjacentVertices*sizeof(double));
	int blockSize = (*ca).getIdealBlockSizeForProblemOfSize(numAdjacentVertices);
	unsigned long numBlocks = std::max<unsigned long>(1, numAdjacentVertices / blockSize);
	std::cout << "getEdgeLengths<<<" << numBlocks << ", " << blockSize <<">>(" << numAdjacentVertices << ")" << std::endl;
	kernel_getEdgeLengths<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, flat_adjacentVertices, adjacentVertices_runLength, vertices, edgeLengths);
	cudaDeviceSynchronize();	//wait for GPU to finish before accessing on host
}

__global__
void kernel_getEdgeLengths(unsigned long numAdjacentVertices, unsigned long numVertices, unsigned long* flat_adjacentVertices, unsigned long* adjacentVertices_runLength, double* vertices, double* edgeLengths){
	//TODO Optimization analysis: storage vs speed
	//this:
	//	flat_adjacentVertices = 6nV (average 6 pairs per vertex)
	//	adjacentVertices_runLength = 1nV
	//	index search requires averagePairCount per Vertex (6nV)
	//fully indexed:
	//	flat_adjacentVertices = 3*6nV (can be halved if redundant AVs are not stored)
	//	no runLength required
	//	no index search time
	unsigned long global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	unsigned long stride = blockDim.x * gridDim.x; //32*3 = 96

	// Use all availble threads to do all numAdjacentVertices
	for(unsigned long av = global_threadIndex; av < numAdjacentVertices; av += stride){
		unsigned long vi = flat_adjacentVertices[av];
		unsigned long v0 = getV0FromRunLength(numVertices, av, adjacentVertices_runLength);
		edgeLengths[av] = cuda_l2norm_diff(vi, v0, vertices);
		//if(v0 == numVertices-1){//0){ //% 1000 == 0){
		//	printf("edgeLengths[%lu] v0 vi %f %lu %lu\n", av, edgeLengths[av], v0, vi);
		//}
		//printf("edgeLength[%d]\t(v0 %d, vi %d)\t%g\n", av, v0, vi, edgeLengths[av]);
	}
}

__device__
unsigned long getV0FromRunLength(unsigned long numVertices, unsigned long av, unsigned long* adjacentVertices_runLength){
	//TODO: measure performance	
	//this: 
	//	pros, smaller memory, 
	//	cons, need this loop to determine v0! (do intelligent search instead)
	//alternatively: save v0 as a second value per index of flat_adjacentVertices
	//	pros, v0 is always known
	//	cons flat_adjacentVertices doubles in size
	unsigned long v0;
	for(unsigned long v = 0; v < numVertices; v++){
		if(av < adjacentVertices_runLength[v]){
			//printf("[%d, %d, %d, %d]:", blockIndex, local_threadIndex, global_threadIndex, av);
			v0 = v;
			break;
		}
	}
	return v0;
}

__device__
double cuda_l2norm_diff(unsigned long vi, unsigned long v0, double* vertices){
	// Too slow
	//if(v0 == 0){ //% 1000 == 0){
	//	printf("vertices[(vi*3)+0], vertices[(v0*3)+0], v0, vi %f %f %lu %lu\n", vertices[(vi*3)+0], vertices[(v0*3)+0], vi, v0);
	//}
	return sqrt((double) (vertices[(vi*3)+0] - vertices[(v0*3)+0])*(vertices[(vi*3)+0] - vertices[(v0*3)+0])
					   + (vertices[(vi*3)+1] - vertices[(v0*3)+1])*(vertices[(vi*3)+1] - vertices[(v0*3)+1])
					   + (vertices[(vi*3)+2] - vertices[(v0*3)+2])*(vertices[(vi*3)+2] - vertices[(v0*3)+2]));
	/* Even slower...!?
	unsigned long vi30 = (vi * 3);
	unsigned long vi31 = (vi * 3) + 1;
	unsigned long vi32 = (vi * 3) + 2;
	unsigned long v030 = (v0 * 3);
	unsigned long v031 = (v0 * 3) + 1;
	unsigned long v032 = (v0 * 3) + 2;
	return sqrt((double) (vertices[vi30] - vertices[v030]) * (vertices[vi30] - vertices[v030])
					   + (vertices[vi31] - vertices[v031]) * (vertices[vi31] - vertices[v031])
					   + (vertices[vi32] - vertices[v032]) * (vertices[vi32] - vertices[v032]));*/
}

void CudaMesh::preCalculateMinEdgeLength(){
	cudaMallocManaged(&minEdgeLength, numVertices*sizeof(double));
	int blockSize = (*ca).getIdealBlockSizeForProblemOfSize(numVertices);
	unsigned long numBlocks = std::max<unsigned long>(1, numVertices / blockSize);
	std::cout << "getMinEdgeLength<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	kernel_getMinEdgeLength<<<numBlocks, blockSize>>>(numAdjacentVertices, numVertices, adjacentVertices_runLength, vertices, edgeLengths, minEdgeLength);
	cudaDeviceSynchronize();
}

__global__
void kernel_getMinEdgeLength(unsigned long numAdjacentVertices, unsigned long numVertices, unsigned long* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength){
	unsigned long global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned long stride = blockDim.x * gridDim.x;
	// Use all availble threads to do all numVertices as v0
	for(unsigned long v0 = global_threadIndex; v0 < numVertices; v0 += stride){
		unsigned long av_begin = (v0 == 0 ? 0 : adjacentVertices_runLength[v0-1]);
		for(unsigned long av = av_begin; av < adjacentVertices_runLength[v0]; av++){
			//if(global_threadIndex < 1){ //% 1000 == 0){
			//	printf("edgeLengths[%d] %f\n", av, edgeLengths[av]);
			//}
			if(minEdgeLength[v0] <= 0 || (edgeLengths[av] > 0 && edgeLengths[av] < minEdgeLength[v0])){
				minEdgeLength[v0] = edgeLengths[av];
			}
		}
		//if(v0 == numVertices-1){//0){//global_threadIndex < 1){ //% 1000 == 0){
		//	printf("minEdgeLength[%d] %f\n", v0, minEdgeLength[v0]);
		//}
	}
}

//TODO: Can do faster with GPU
void CudaMesh::preCalculateGlobalMinEdgeLength(){
	for(unsigned long vi = 0; vi < numVertices; vi++){
		if(globalMinEdgeLength <= 0 || (minEdgeLength[vi] > 0 && minEdgeLength[vi] < globalMinEdgeLength)){
			globalMinEdgeLength = minEdgeLength[vi];
		}
	}
	printf("globalMinEdgeLength found to be %f\n", globalMinEdgeLength);
}

void CudaMesh::calculateOneRingMeanFunctionValues(){
	cudaMallocManaged(&oneRingMeanFunctionValues, numVertices*sizeof(double));
	int blockSize = (*ca).getIdealBlockSizeForProblemOfSize(numVertices);
	unsigned long numBlocks = std::max<unsigned long>(1, numVertices / blockSize);
	std::cout << "getOneRingMeanFunctionValues<<<" << numBlocks << ", " << blockSize << ">>(" << numVertices << ")" << std::endl;
	kernel_getOneRingMeanFunctionValues<<<numBlocks, blockSize>>>(
		numVertices, 
		adjacentVertices_runLength, 
		facesOfVertices_runLength, 
		flat_facesOfVertices, 
		flat_adjacentVertices, 
		faces,
		minEdgeLength,
		globalMinEdgeLength,
		functionValues,
		edgeLengths,
		oneRingMeanFunctionValues
	);
	cudaDeviceSynchronize();
}

__global__
void kernel_getOneRingMeanFunctionValues(
	unsigned long numVertices, 
	unsigned long* adjacentVertices_runLength,
	unsigned long* facesOfVertices_runLength, 
	unsigned long* flat_facesOfVertices, 
	unsigned long* flat_adjacentVertices,
	unsigned long* faces, 
	double* minEdgeLength, 
	double globalMinEdgeLength, 
	double* functionValues,
	double* edgeLengths,
	double* oneRingMeanFunctionValues
){
	unsigned long global_threadIndex = blockIdx.x * blockDim.x + threadIdx.x; //0-95
	unsigned long stride = blockDim.x * gridDim.x; //32*3 = 96

	double accuFuncVals = 0.0;
	double accuArea = 0.0;

	// Use all availble threads to do all numVertices as v0
	for(unsigned long v0 = global_threadIndex; v0 < numVertices; v0 += stride){
		unsigned long fi_begin = (v0 == 0 ? 0 : facesOfVertices_runLength[v0-1]);
		for(unsigned long fi = fi_begin; fi < facesOfVertices_runLength[v0]; fi++){
			//currFace->getFuncVal1RingSector( this, rMinDist, currArea, currFuncVal ); //ORS.307
				//get1RingSectorConst();
				unsigned long vi, vip1;
				getViAndVip1FromV0andFi(v0, flat_facesOfVertices[fi], faces, vi, vip1);
				//printf("[%d]\t[%d]\t%d\t%d\n", v0, flat_facesOfVertices[fi], vi, vip1);

				//TODO: Ensure edges A, B, C are correct with v0, vi, vip1; also regarding funcVals later
				//ORS.456
				double lengthEdgeA = getEdgeLengthOfV0AndVi(vi, vip1, adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double lengthEdgeB = getEdgeLengthOfV0AndVi(v0, vip1, adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double lengthEdgeC = getEdgeLengthOfV0AndVi(v0, vi,   adjacentVertices_runLength, flat_adjacentVertices, edgeLengths);
				double alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2*lengthEdgeB*lengthEdgeC ) );

				double rNormDist = globalMinEdgeLength; //minEdgeLength[v0];
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
				double funcValCenter = functionValues[v0];
				double funcValA = functionValues[vi];
				double funcValB = functionValues[vip1];

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

			if(v0 == numVertices-1){//0){//global_threadIndex < 1){ //% 1000 == 0){
				printf("v0, rNormDist, currFuncVal, currArea %d %f %f %f\n", v0, rNormDist, currFuncVal, currArea);
			}
		}

		oneRingMeanFunctionValues[v0] = accuFuncVals / accuArea;
		if(v0 == numVertices-1){//){//global_threadIndex < 1){ //% 1000 == 0){
			printf("v0, accuFuncVals, accuArea %d %f %f\n", v0, accuFuncVals, accuArea);
			printf("oneRingMeanFunctionValues[%d] %f\n", v0, oneRingMeanFunctionValues[v0]);
		}
	}
}

__device__
void getViAndVip1FromV0andFi(unsigned long v0, unsigned long fi, unsigned long* faces, unsigned long& vi, unsigned long& vip1){
	//printf("faces[%d*3+{0,1,2}] {%d, %d, %d}\n", fi, faces[(fi*3)+0], faces[(fi*3)+1], faces[(fi*3)+2]);
	bool isViAssigned = false;
	for(int i = 0; i < 3; i++){ // for each vertex in this face (a, b, c)
		unsigned long v = faces[fi*3+i];
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
double getEdgeLengthOfV0AndVi(unsigned long v0, unsigned long vi, unsigned long* adjacentVertices_runLength, unsigned long* flat_adjacentVertices, double* edgeLengths){
	//TODO: Error handling?
	unsigned long av_begin = (v0 == 0 ? 0 : adjacentVertices_runLength[v0-1]);
	double edgeLength;
	for(unsigned long av = av_begin; av < adjacentVertices_runLength[v0]; av++){
		if(flat_adjacentVertices[av] == vi){
			edgeLength = edgeLengths[av];
			break;
		}
	}
	return edgeLength;
}

