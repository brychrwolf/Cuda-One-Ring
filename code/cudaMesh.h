#ifndef CUDAMESH_H
#define CUDAMESH_H

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

class CudaMesh{
		//int deviceCount;

	public:
		CudaMesh();

		void loadPLY(std::string fileName, int& numVertices, double** vertices, double** featureVectors, int& numFaces, int** faces);
		void printMesh(int numVertices, double* vertices, double* featureVectors, int numFaces, int* faces);
		void printVariableDepth2dArrayOfSets(std::string name, std::set<int> arrayOfSets[], int size);
};


#endif // CUDAMESH_H

