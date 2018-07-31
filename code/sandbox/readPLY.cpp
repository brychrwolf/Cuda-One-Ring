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
	
	int v_idx = 0;
	int x_idx;
	int y_idx;
	int z_idx;

	std::string line;
	int lineNumber = 0;
	// read every line in the file
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
				}else{
					std::cerr << "ERR (" << lineNumber << "): Bad Element:: " << line << std::endl;
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
				flat_vertices = (int*) malloc(3 * numVertices * sizeof(int));
				flat_faces = (int*) malloc(3 * numFaces * sizeof(int));
			}
		}else if(lineNumber < faceSectionBegin){
			std::vector<int> coords = split<int>(line);
			flat_vertices[vi*3 + 0] = coords[x_idx];
			flat_vertices[vi*3 + 1] = coords[y_idx];
			flat_vertices[vi*3 + 2] = coords[z_idx];
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
	
	for(vi = 0; vi < numVertices; vi++)
		std::cerr << "v[" << vi << "] " << flat_vertices[vi*3 + 0] << " " << flat_vertices[vi*3 + 1] << " " << flat_vertices[vi*3 + 2] << std::endl;
	std::cerr << std::endl;
	for(fi = 0; fi < numFaces; fi++)
		std::cerr << "f[" << fi << "] " << flat_faces[fi*3 + 0] << " " << flat_faces[fi*3 + 1] << " " << flat_faces[fi*3 + 2] << std::endl;
	std::cerr << std::endl << "numVertices " << numVertices << " numFaces " << numFaces << std::endl;
}


