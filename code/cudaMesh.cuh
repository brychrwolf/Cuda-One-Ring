#ifndef CUDAMESH_CUH
#define CUDAMESH_CUH

#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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

// Kernels must be defined outside of the class
__global__ void kernel_getEdgeLengths(int numAdjacentVertices, int numVertices, int* flat_adjacentVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths);
__device__ int getV0FromRunLength(int numVertices, int av, int* adjacentVertices_runLength);
__device__ double cuda_l2norm_diff(int vi, int v0, double* vertices);
__global__ void kernel_getMinEdgeLength(int numAdjacentVertices, int numVertices, int* adjacentVertices_runLength, double* vertices, double* edgeLengths, double* minEdgeLength);

class CudaMesh{
		int numVertices;
		int numFaces;
		double* vertices;
		double* featureVectors;
		int* faces;
		std::vector<std::set<int>> adjacentVertices;
		std::vector<std::set<int>> facesOfVertices;
		int* adjacentVertices_runLength;
		int* facesOfVertices_runLength;
		int numAdjacentVertices;
		int numFacesOfVertices;
		int* flat_adjacentVertices;
		int* flat_facesOfVertices;
		double* edgeLengths;
		double* minEdgeLength;

	public:
		CudaMesh();
		~CudaMesh();
		
		/* Getters and Setters */
		int getNumVertices();
		int getNumFaces();
		double* getVertices();
		double* getFeatureVectors();
		int* getFaces();
		std::vector<std::set<int>> getAdjacentVertices();
		std::vector<std::set<int>> getFacesOfVertices();
		int* getAdjacentVertices_runLength();
		int* getFacesOfVertices_runLength();
		int getNumAdjacentVertices();
		int getNumFacesOfVertices();
		int* getFlat_adjacentVertices();
		int* getFlat_facesOfVertices();
		double* getEdgeLengths();
		double* getMinEdgeLength();
		
		void setNumVertices(int upd);
		void setNumFaces(int upd);
		void setVertices(double* upd);
		void setFeatureVectors(double* upd);
		void setFaces(int* upd);
		void setAdjacentVertices(std::vector<std::set<int>> upd);
		void setFacesOfVertices(std::vector<std::set<int>> upd);
		void setAdjacentVertices_runLength(int* upd);
		void setFacesOfVertices_runLength(int* upd);
		void setNumAdjacentVertices(int upd);
		void setNumFacesOfVertices(int upd);
		void setFlat_adjacentVertices(int* upd);
		void setFlat_facesOfVertices(int* upd);
		void setEdgeLengths(double* upd);
		void setMinEdgeLength(double* upd);
		
		/* IO */
		void loadPLY(std::string fileName);
		void printMesh();
		void printFacesOfVertices();
		void printAdjacentVertices();
		void printFacesOfVertices_RunLength();
		void printAdjacentVertices_RunLength();
		void printFlat_AdjacentVertices();
		void printFlat_FacesOfVertices();
		void printEdgeLengths();
		void printMinEdgeLength();
		
		/* Build Tables */
		void buildSets();
		void determineRunLengths();
		void flattenSets();
		
		/* Pre-Calculate */
		void preCalculateEdgeLengths();
		void preCalculateMinEdgeLength();
};

#endif // CUDAMESH_CUH

