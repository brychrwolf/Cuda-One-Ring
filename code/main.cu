#include <stdio.h>

#include <array>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <map>

#include "cudaAccess.cuh"
#include "cudaMesh.cuh"
#include "cudaTimer.cuh"

// to engage GPUs when installed in hybrid system, run as 
// optirun ./main

__global__ void getOneRingMeanFunctionValues(
	int numVertices, 
	int* adjacentVertices_runLength,
	int* facesOfVertices_runLength, 
	int* flat_facesOfVertices, 
	int* flat_adjacentVertices,
	int* faces, 
	double* minEdgeLength, 
	double* featureVectors, 
	double* edgeLengths,
	double* oneRingMeanFunctionValues
);
__device__ void getViAndVip1FromV0andFi(int v0, int fi, int* faces, int& vi, int& vip1);
__device__ double getEdgeLengthOfV0AndVi(int v0, int vi, int* adjacentVertices_runLength, int* flat_adjacentVertices, double* edgeLengths);

void printOneRingMeanFunctionValues(int numVertices, double* oneRingMeanFunctionValues);

int main(int ac, char** av){
	/*************************************************************************/
	std::cout << std::endl << "****** Initializing CUDA..." << std::endl;
	/*************************************************************************/
	CudaAccess ca;

	printf("CUDA Device Query...\n");
	if(ca.getDeviceCount() <= 0){
		std::cout << "No CUDA devices found." << std::endl;
		std::cout << "TERMINATING NOW." << std::endl; //TODO: Support nonGPU?
		return 0;
	}else{
		ca.printCUDAProps();
	}
	
	CudaTimer ct_LoadingMesh;
	CudaTimer ct_BuildingTables;
		CudaTimer ct_BuildingSets;
		CudaTimer ct_DetermineRunLengths;
		CudaTimer ct_FlattenSets;
		CudaTimer ct_PreCalEdgeLengths;
		CudaTimer ct_preCalMinEdgeLength;
	CudaTimer ct_Calculating;

	/*************************************************************************/
	std::cout << "****** CUDA Initialized." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Loading Mesh..." << std::endl;
	/*************************************************************************/
	CudaMesh cm(&ca);
	
	ct_LoadingMesh.start();
	cm.loadPLY(av[1]); //TODO: add error handeling for when av[1] is not a valid filename
	//cm.loadPLY("../example_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply");
	//cm.loadPLY("../example_meshes/h.ply");
	ct_LoadingMesh.stop();
	
	//cm.printMesh();	
	std::cout << "numVertices " << cm.getNumVertices() << " numFaces " << cm.getNumFaces() << std::endl;
	/*************************************************************************/
	std::cout << "****** Finished Loading." << std::endl;
	/*************************************************************************/


	
	/*************************************************************************/
	std::cout << std::endl << "****** Begin Building Tables..." << std::endl;
	/*************************************************************************/
	ct_BuildingTables.start();
	std::cout << "Building set of faces by vertex, " << std::endl;
	std::cout << "and table of adjacent vertices by vertex..." << std::endl;
	ct_BuildingSets.start();
	cm.buildSets();
	ct_BuildingSets.stop();
	//cm.printAdjacentVertices();
	//cm.printFacesOfVertices();
	
	std::cout << "Determine runlengths of adjacentVertices and facesofVertices" << std::endl;
	ct_DetermineRunLengths.start();
	cm.determineRunLengths();
	ct_DetermineRunLengths.stop();
	//cm.printAdjacentVertices_RunLength();
	//cm.printFacesOfVertices_RunLength();
	
	std::cout << "Flatten adjacentVerticies and facesOfVertices" << std::endl;
	ct_FlattenSets.start();
	cm.flattenSets();
	ct_FlattenSets.stop();
	//cm.printFlat_AdjacentVertices();
	//cm.printFlat_FacesOfVertices();
	
	std::cout << "Precalculate Edge Lengths" << std::endl;
	ct_PreCalEdgeLengths.start();
	cm.preCalculateEdgeLengths();
	ct_PreCalEdgeLengths.stop();
	//cm.printEdgeLengths();
	
	std::cout << "Precalculate minimum edge length among adjacent vertices..." << std::endl;
	ct_preCalMinEdgeLength.start();
	cm.preCalculateMinEdgeLength();
	ct_preCalMinEdgeLength.stop();
	//cm.printMinEdgeLength();
	
	ct_BuildingTables.stop();
	/*************************************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/*************************************************************************/
	ct_Calculating.start();
	std::cout << "Calculating oneRingMeanFunctionValues (circle sectors)..." << std::endl;
	double* oneRingMeanFunctionValues;
	cudaMallocManaged(&oneRingMeanFunctionValues, cm.getNumVertices()*sizeof(double));
	int blockSize = ca.getIdealBlockSizeForProblemOfSize(cm.getNumVertices());
	int numBlocks = max(1, cm.getNumVertices() / blockSize);
	std::cout << "getOneRingMeanFunctionValues<<<" << numBlocks << ", " << blockSize << ">>(" << cm.getNumVertices() << ")" << std::endl;
	getOneRingMeanFunctionValues<<<numBlocks, blockSize>>>(
		cm.getNumVertices(), 
		cm.getAdjacentVertices_runLength(), 
		cm.getFacesOfVertices_runLength(), 
		cm.getFlat_facesOfVertices(), 
		cm.getFlat_adjacentVertices(), 
		cm.getFaces(),
		cm.getMinEdgeLength(),
		cm.getFeatureVectors(),
		cm.getEdgeLengths(),
		oneRingMeanFunctionValues
	);
	cudaDeviceSynchronize();
	//printOneRingMeanFunctionValues(cm.getNumVertices(), oneRingMeanFunctionValues);
	ct_Calculating.stop();
	/*************************************************************************/
	std::cout << "****** Finished Calculating." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Analyzing..." << std::endl;
	/*************************************************************************/
	std::cout << "Elapsed times:" << std::endl;
	std::cout << "LoadingMesh\t" << ct_LoadingMesh.getElapsedTime() << std::endl;
	std::cout << "BuildingTables\t" << ct_BuildingTables.getElapsedTime() << std::endl;
	std::cout << "\tBuildingSets\t\t" << ct_BuildingSets.getElapsedTime() << std::endl;
	std::cout << "\tDetermineRunLengths\t" << ct_DetermineRunLengths.getElapsedTime() << std::endl;
	std::cout << "\tFlattenSets\t\t" << ct_FlattenSets.getElapsedTime() << std::endl;
	std::cout << "\tPreCalEdgeLengths\t" << ct_PreCalEdgeLengths.getElapsedTime() << std::endl;
	std::cout << "\tPreCalMinEdgeLength\t" << ct_preCalMinEdgeLength.getElapsedTime() << std::endl;
	std::cout << "Calculating\t" << ct_Calculating.getElapsedTime() << std::endl;
	/*************************************************************************/
	std::cout << "****** Finished Analyzing..." << std::endl;
	/*************************************************************************/
}

__global__
void getOneRingMeanFunctionValues(
	int numVertices, 
	int* adjacentVertices_runLength,
	int* facesOfVertices_runLength, 
	int* flat_facesOfVertices, 
	int* flat_adjacentVertices,
	int* faces, 
	double* minEdgeLength, 
	double* featureVectors, 
	double* edgeLengths,
	double* oneRingMeanFunctionValues
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

		oneRingMeanFunctionValues[v0] = accuFuncVals / accuArea;
		//if(global_threadIndex % 1000 == 0)
		//	printf("oneRingMeanFunctionValues[%d] %f\n", v0, oneRingMeanFunctionValues[v0]);
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

void printOneRingMeanFunctionValues(int numVertices, double* oneRingMeanFunctionValues){
	std::cerr << std::endl;
	for(int i = 0; i < numVertices; i++){
		std::cerr << "oneRingMeanFunctionValues[" << i << "] " << oneRingMeanFunctionValues[i] << std::endl;
	}
}

