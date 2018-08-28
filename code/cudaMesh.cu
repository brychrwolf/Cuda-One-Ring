#include <iostream>
#include <fstream>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "cudaMesh.h"

//TODO: Only loads PLY files, should support other file types!

CudaMesh::CudaMesh(){
	//updateDeviceCount();
}

void CudaMesh::loadPLY(std::string fileName, int& numVertices, double** vertices, double** featureVectors, int& numFaces, int** faces){
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
			(*featureVectors)[vi] = dis(gen);//1;
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

void CudaMesh::printMesh(
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



void CudaMesh::printVariableDepth2dArrayOfSets(std::string name, std::set<int> arrayOfSets[], int size){
	std::cerr << std::endl;
	for(int i = 0; i < size; i++){
		std::cerr << name << "[" << i << "] ";
		for(int elem : arrayOfSets[i])
			std::cerr << elem << " ";
		std::cerr << std::endl;
	}
}
