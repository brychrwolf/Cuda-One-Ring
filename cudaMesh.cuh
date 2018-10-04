#ifndef CUDAMESH_CUH
#define CUDAMESH_CUH

#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "cudaAccess.cuh"

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
template std::vector<unsigned long> split<unsigned long>(std::string);

// Kernels must be defined outside of the class
__global__ void kernel_getEdgeLengths(unsigned long numAdjacentVertices, unsigned long numVertices, unsigned long* flat_adjacentVertices, unsigned long* adjacentVertices_runLength, double* vertices, double* edgeLengths);
__device__ unsigned long getV0FromRunLength(unsigned long numVertices, unsigned long av, unsigned long* adjacentVertices_runLength);
__device__ double cuda_l2norm_diff(unsigned long vi, unsigned long v0, double* vertices);
__global__ void kernel_getMinEdgeLength(unsigned long numAdjacentVertices, unsigned long numVertices, unsigned long* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength);
__global__ void kernel_getOneRingMeanFunctionValues(
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
);
__device__ void getViAndVip1FromV0andFi(unsigned long v0, unsigned long fi, unsigned long* faces, unsigned long& vi, unsigned long& vip1);
__device__ double getEdgeLengthOfV0AndVi(unsigned long v0, unsigned long vi, unsigned long* adjacentVertices_runLength, unsigned long* flat_adjacentVertices, double* edgeLengths);

class CudaMesh{
		CudaAccess* ca;
		unsigned long numVertices;
		unsigned long numFaces;
		double* vertices;
		double* functionValues;
		unsigned long* faces;
		std::vector<std::set<unsigned long>> adjacentVertices;
		std::vector<std::set<unsigned long>> facesOfVertices;
		unsigned long* adjacentVertices_runLength;
		unsigned long* facesOfVertices_runLength;
		unsigned long numAdjacentVertices;
		unsigned long numFacesOfVertices;
		unsigned long* flat_adjacentVertices;
		unsigned long* flat_facesOfVertices;
		double* edgeLengths;
		double* minEdgeLength;
		double globalMinEdgeLength;
		double* oneRingMeanFunctionValues;

	public:
		CudaMesh();
		CudaMesh(CudaAccess* acc);
		~CudaMesh();
		
		/* Getters and Setters */
		unsigned long getNumVertices();
		unsigned long getNumFaces();
		double* getVertices();
		double* getFunctionValues();
		unsigned long* getFaces();
		std::vector<std::set<unsigned long>> getAdjacentVertices();
		std::vector<std::set<unsigned long>> getFacesOfVertices();
		unsigned long* getAdjacentVertices_runLength();
		unsigned long* getFacesOfVertices_runLength();
		unsigned long getNumAdjacentVertices();
		unsigned long getNumFacesOfVertices();
		unsigned long* getFlat_adjacentVertices();
		unsigned long* getFlat_facesOfVertices();
		double* getEdgeLengths();
		double* getMinEdgeLength();
		double getGlobalMinEdgeLength();
		double* getOneRingMeanFunctionValues();
		
		void setNumVertices(unsigned long upd);
		void setNumFaces(unsigned long upd);
		void setVertices(double* upd);
		void setFunctionValues(double* upd);
		void setFaces(unsigned long* upd);
		void setAdjacentVertices(std::vector<std::set<unsigned long>> upd);
		void setFacesOfVertices(std::vector<std::set<unsigned long>> upd);
		void setAdjacentVertices_runLength(unsigned long* upd);
		void setFacesOfVertices_runLength(unsigned long* upd);
		void setNumAdjacentVertices(unsigned long upd);
		void setNumFacesOfVertices(unsigned long upd);
		void setFlat_adjacentVertices(unsigned long* upd);
		void setFlat_facesOfVertices(unsigned long* upd);
		void setEdgeLengths(double* upd);
		void setMinEdgeLength(double* upd);
		void setGlobalMinEdgeLength(double upd);
		void setOneRingMeanFunctionValues(double* upd);
		
		/* IO */
		void loadPLY(std::string fileName);
		void loadFunctionValues(std::string fileName);
		void writeFunctionValues(std::string fileName);
		void printMesh();
		void printFacesOfVertices();
		void printAdjacentVertices();
		void printFacesOfVertices_RunLength();
		void printAdjacentVertices_RunLength();
		void printFlat_AdjacentVertices();
		void printFlat_FacesOfVertices();
		void printEdgeLengths();
		void printMinEdgeLength();
		void printOneRingMeanFunctionValues();
		
		/* Build Tables */
		void buildSets();
		void determineRunLengths();
		void flattenSets();
		void freeSets();
		
		/* Pre-Calculate */
		void preCalculateEdgeLengths();
		void preCalculateMinEdgeLength();
		void preCalculateGlobalMinEdgeLength();
		
		/* Calculate */
		void calculateOneRingMeanFunctionValues();
};

#endif // CUDAMESH_CUH

