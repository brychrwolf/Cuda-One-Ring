#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>

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

int main(){
	//std::ifstream infile("test.ply");
	std::ifstream infile("../../example_meshes/h.ply");
	//std::ifstream infile("../../example_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply");

	bool inHeaderSection = true;
	int faceSectionBegin;
	int vi = 0;
	int fi = 0;
	
	int numVertices;
	int numFaces;
	
	int* flat_vertices;
	int* flat_faces;

	std::string line;
	int lineNumber = 0;
	while(std::getline(infile, line)){
		if(inHeaderSection){
			if(line.substr(0, 7) == "element"){
				if(line.substr(8, 6) == "vertex"){
					std::vector<std::string> words = split<std::string>(line);
					std::istringstream convert(words[2]);
					convert >> numVertices;
					flat_vertices = (int*) malloc(3 * numVertices * sizeof(int));
				}else if(line.substr(8, 4) == "face"){
					std::vector<std::string> words = split<std::string>(line);
					std::istringstream convert(words[2]);
					convert >> numFaces;
					flat_faces = (int*) malloc(3 * numFaces * sizeof(int));
				}else{
					std::cerr << "ERR (" << lineNumber << "): Bad Element:: " << line << std::endl;
				}
			}else if(line.substr(0, 8) == "property"){
				//parseProperty(line);
			}else if(line.substr(0, 10) == "end_header"){
				inHeaderSection = false;
				faceSectionBegin = lineNumber + 1 + numVertices;
			}
		}else if(lineNumber < faceSectionBegin){
			std::vector<int> coords = split<int>(line);
			flat_vertices[vi*3 + 0] = coords[0];
			flat_vertices[vi*3 + 1] = coords[1];
			flat_vertices[vi*3 + 2] = coords[2];
			vi++;
		}else{
			std::vector<int> coords = split<int>(line);
			flat_faces[fi*3 + 0] = coords[1]; //coords[0] is list size
			flat_faces[fi*3 + 1] = coords[2];
			flat_faces[fi*3 + 2] = coords[3];
			fi++;
		}
		lineNumber++;
	}
	
	for(vi = 0; vi < numVertices; vi++){
		std::cerr << "v[" << vi << "] " << flat_vertices[vi*3 + 0] << " " << flat_vertices[vi*3 + 1] << " " << flat_vertices[vi*3 + 2] << std::endl;
	}
	
	for(fi = 0; fi < numFaces; fi++){
		std::cerr << "f[" << fi << "] " << flat_faces[fi*3 + 0] << " " << flat_faces[fi*3 + 1] << " " << flat_faces[fi*3 + 2] << std::endl;
	}
	
	std::cerr << std::endl << "numVertices " << numVertices << " numFaces " << numFaces << std::endl;
}


