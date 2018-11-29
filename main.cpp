#include <stdio.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

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
	CudaTimer timer_PreCalculations;
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

	std::string plyFileName;
	std::string plyDirectory = "";
	if(ac > 1)
		plyFileName = av[1];
	else if(ac > 2)
		plyDirectory = av[2];
	else{
		plyFileName = "Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply";
		//plyFileName = "h.ply";
		plyDirectory = "example_meshes";
	}
	std::string fileNameBase = plyFileName.substr(0, plyFileName.length()-4);
	std::string funcValsFileName = fileNameBase+"_funcvals.txt";

	timer_LoadingMesh.start();
	//TODO: fail on no load
	cm.loadPLY(plyDirectory+"/"+plyFileName);
	cm.loadFunctionValues(plyDirectory+"/"+funcValsFileName);
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

	std::cout << "Free non-flat sets" << std::endl;
	cm.freeSets();

	timer_BuildingTables.stop();
	/*************************************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Pre-Calculating..." << std::endl;
	/*************************************************************************/
	timer_PreCalculations.start();
	std::cout << "Precalculate Edge Lengths" << std::endl;
	timer_PreCalEdgeLengths.start();
	cm.preCalculateEdgeLengths();
	timer_PreCalEdgeLengths.stop();
	//ca.printMemInfo();
	//ca.printLastCUDAError();
	//cm.printEdgeLengths();

	std::cout << "Precalculate minimum edge length among adjacent vertices..." << std::endl;
	timer_preCalMinEdgeLength.start();
	cm.preCalculateMinEdgeLength();
	cm.preCalculateGlobalMinEdgeLength(); //TODO: Add own timer
	timer_preCalMinEdgeLength.stop();
	//ca.printMemInfo();
	//ca.printLastCUDAError();
	//cm.printMinEdgeLength();

	timer_PreCalculations.stop();
	/*************************************************************************/
	std::cout << "****** Finished Building Tables." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/*************************************************************************/
	std::cout << "Calculating oneRingMeanFunctionValues (circle sectors)..." << std::endl;
	//ca.printMemInfo();
	timer_Calculating.start();
	cm.calculateOneRingMeanFunctionValues();
	timer_Calculating.stop();
	//cm.printOneRingMeanFunctionValues();
	//ca.printMemInfo();
	ca.printLastCUDAError();
	/*************************************************************************/
	std::cout << "****** Finished Calculating." << std::endl;
	/*************************************************************************/



	/*************************************************************************/
	std::cout << std::endl << "****** Begin Analyzing..." << std::endl;
	/*************************************************************************/
	cm.writeFunctionValues(plyDirectory+"/experiments/"+fileNameBase+"_funcvals_1iter_libcudaonering.txt");
	cm.analyzeFunctionValues(plyDirectory+"/experiments/"+fileNameBase+"_funcvals_1iter_trimmed.txt", 1e-5);

	std::cout << std::endl << "Elapsed times:" << std::endl;
	std::cout << "LoadingMesh\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_LoadingMesh.getElapsedTime() << std::endl;
	std::cout << "BuildingTables\t" << std::fixed << std::setw(10) << std::setprecision(3) << timer_BuildingTables.getElapsedTime() << std::endl;
	std::cout << "   BuildingSets\t\t" 		<< std::fixed << std::setw(10) << std::setprecision(3) << timer_BuildingSets.getElapsedTime() << std::endl;
	std::cout << "   DetermineRunLengths\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_DetermineRunLengths.getElapsedTime() << std::endl;
	std::cout << "   FlattenSets\t\t" 		<< std::fixed << std::setw(10) << std::setprecision(3) << timer_FlattenSets.getElapsedTime() << std::endl;
	std::cout << "PreCalculations\t" << std::fixed << std::setw(10) << std::setprecision(3) << timer_PreCalculations.getElapsedTime() << std::endl;
	std::cout << "   PreCalEdgeLengths\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_PreCalEdgeLengths.getElapsedTime() << std::endl;
	std::cout << "   PreCalMinEdgeLength\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_preCalMinEdgeLength.getElapsedTime() << std::endl;
	std::cout << "Calculating\t" 	<< std::fixed << std::setw(10) << std::setprecision(3) << timer_Calculating.getElapsedTime() << std::endl;
	/*************************************************************************/
	std::cout << "****** Finished Analyzing..." << std::endl;
	/*************************************************************************/
}

