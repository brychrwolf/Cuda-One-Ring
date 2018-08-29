#include <stdio.h>

#include <array>
#include <cmath>
#include <cfloat>
#include <iomanip>
#include <iostream>
#include <map>

#include "cudaAccess.cuh"
#include "cudaMesh.cuh"
#include "cudaTimer.cuh"

// to engage GPUs when installed in hybrid system, run as 
// optirun ./main

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
	
	CudaTimer timer_LoadingMesh;
	CudaTimer timer_BuildingTables;
		CudaTimer timer_BuildingSets;
		CudaTimer timer_DetermineRunLengths;
		CudaTimer timer_FlattenSets;
		CudaTimer timer_PreCalEdgeLengths;
		CudaTimer timer_preCalMinEdgeLength;
	CudaTimer timer_Calculating;

	/*************************************************************************/
	std::cout << "****** CUDA Initialized." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Loading Mesh..." << std::endl;
	/*************************************************************************/
	CudaMesh cm(&ca);
	
	timer_LoadingMesh.start();
	cm.loadPLY(av[1]); //TODO: add error handeling for when av[1] is not a valid filename
	//cm.loadPLY("../example_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply");
	//cm.loadPLY("../example_meshes/h.ply");
	timer_LoadingMesh.stop();
	
	//cm.printMesh();	
	std::cout << "numVertices " << cm.getNumVertices() << " numFaces " << cm.getNumFaces() << std::endl;
	/*************************************************************************/
	std::cout << "****** Finished Loading." << std::endl;
	/*************************************************************************/


	
	/*************************************************************************/
	std::cout << std::endl << "****** Begin Building Tables..." << std::endl;
	/*************************************************************************/
	timer_BuildingTables.start();
	std::cout << "Building set of faces by vertex, " << std::endl;
	std::cout << "and table of adjacent vertices by vertex..." << std::endl;
	timer_BuildingSets.start();
	cm.buildSets();
	timer_BuildingSets.stop();
	//cm.printAdjacentVertices();
	//cm.printFacesOfVertices();
	
	std::cout << "Determine runlengths of adjacentVertices and facesofVertices" << std::endl;
	timer_DetermineRunLengths.start();
	cm.determineRunLengths();
	timer_DetermineRunLengths.stop();
	//cm.printAdjacentVertices_RunLength();
	//cm.printFacesOfVertices_RunLength();
	
	std::cout << "Flatten adjacentVerticies and facesOfVertices" << std::endl;
	timer_FlattenSets.start();
	cm.flattenSets();
	timer_FlattenSets.stop();
	//cm.printFlat_AdjacentVertices();
	//cm.printFlat_FacesOfVertices();
	
	std::cout << "Precalculate Edge Lengths" << std::endl;
	timer_PreCalEdgeLengths.start();
	cm.preCalculateEdgeLengths();
	timer_PreCalEdgeLengths.stop();
	//cm.printEdgeLengths();
	
	std::cout << "Precalculate minimum edge length among adjacent vertices..." << std::endl;
	timer_preCalMinEdgeLength.start();
	cm.preCalculateMinEdgeLength();
	timer_preCalMinEdgeLength.stop();
	//cm.printMinEdgeLength();
	
	timer_BuildingTables.stop();
	/*************************************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/*************************************************************************/
	timer_Calculating.start();
	std::cout << "Calculating oneRingMeanFunctionValues (circle sectors)..." << std::endl;
	cm.calculateOneRingMeanFunctionValues();
	cm.printOneRingMeanFunctionValues();
	timer_Calculating.stop();
	/*************************************************************************/
	std::cout << "****** Finished Calculating." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Analyzing..." << std::endl;
	/*************************************************************************/
	std::cout << "Elapsed times:" << std::endl;
	std::cout << "LoadingMesh\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_LoadingMesh.getElapsedTime() << std::endl;
	std::cout << "BuildingTables\t" << std::fixed << std::setw(10) << std::setprecision(3) << timer_BuildingTables.getElapsedTime() << std::endl;
	std::cout << "\tBuildingSets\t\t" 		<< std::fixed << std::setw(10) << std::setprecision(3) << timer_BuildingSets.getElapsedTime() << std::endl;
	std::cout << "\tDetermineRunLengths\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_DetermineRunLengths.getElapsedTime() << std::endl;
	std::cout << "\tFlattenSets\t\t" 		<< std::fixed << std::setw(10) << std::setprecision(3) << timer_FlattenSets.getElapsedTime() << std::endl;
	std::cout << "\tPreCalEdgeLengths\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_PreCalEdgeLengths.getElapsedTime() << std::endl;
	std::cout << "\tPreCalMinEdgeLength\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_preCalMinEdgeLength.getElapsedTime() << std::endl;
	std::cout << "Calculating\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_Calculating.getElapsedTime() << std::endl;
	/*************************************************************************/
	std::cout << "****** Finished Analyzing..." << std::endl;
	/*************************************************************************/
}

