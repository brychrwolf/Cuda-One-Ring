#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>

typedef struct {
	int x;
	int y;
	int z;
} vertex;

typedef struct {
	int a;
	int b;
	int c;
} face;

int main(){
	std::cout << "Loading the \"Debossed H\" Mesh..." << std::endl;
	
	std::cout << "Loading Vertexes..." << std::endl;
	int numVerticies = 22;
	vertex verticies[numVerticies] = {
		(vertex) { 0,  0,  0},	(vertex) { 2,  0,  0},
		(vertex) {12,  0,  0},	(vertex) {14,  0,  0},
		(vertex) {14, 20,  0},	(vertex) {12, 20,  0},
		(vertex) { 2, 20,  0},	(vertex) { 0, 20,  0},
		(vertex) { 1,  1, -1},	(vertex) {13,  1, -1},
		(vertex) {13, 19, -1},	(vertex) { 1, 19, -1},
		(vertex) { 2, 10,  0},	(vertex) {12, 10,  0},
		(vertex) {12, 12,  0},	(vertex) { 2, 12,  0},
		(vertex) { 1, 11, -1},	(vertex) {13, 11, -1},
		(vertex) {-2, -2,  0},	(vertex) {16, -2,  0},
		(vertex) {16, 22,  0},	(vertex) {-2, 22,  0}
	};	
	for(int i = 0; i < numVerticies; i++){
		std::cout << i << " = {" << verticies[i].x << ", " << verticies[i].y << ", " << verticies[i].z << "}" <<std::endl;
	}
	
	std::cout << std::endl << "Generating Random Feature Vectors..." << std::endl;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);
	float featureVectors[numVerticies] = {};
	for(int i = 0; i < numVerticies; i++){
		featureVectors[i] = dis(gen);
		std::cout << "featureVector [" << i << "] = " << featureVectors[i] << std::endl;
	}
	
	std::cout << std::endl << "Loading Faces..." << std::endl;
	int numFaces = 36;
	face faces[numFaces] = {
		(face) { 0,  1,  8}, (face) { 1, 16,  8},
		(face) { 1, 12, 16}, (face) {12, 13, 16},
		(face) {13, 17, 16}, (face) { 9, 17, 13},
		(face) { 2,  9, 13}, (face) { 2,  3,  9},
		(face) { 3, 10,  9}, (face) { 3,  4, 10},
		(face) { 4,  5, 10}, (face) { 5, 17, 10},
		(face) { 5, 14, 17}, (face) {14, 15, 17},
		(face) {15, 16, 17}, (face) {11, 16, 15},
		(face) { 6, 11, 15}, (face) { 6,  7, 11},
		(face) { 7,  8, 11}, (face) { 0,  8,  7},
		(face) { 0, 18,  1}, (face) { 1, 18, 19},
		(face) { 1, 19,  2}, (face) { 2, 19,  3},
		(face) { 3, 19,  4}, (face) { 4, 19, 20},
		(face) { 4, 20,  5}, (face) { 5, 20, 21},
		(face) { 5, 21,  6}, (face) { 6, 21,  7},
		(face) { 0,  7, 21}, (face) { 0, 21, 18},
		(face) { 1,  2, 12}, (face) { 2, 13, 12},
		(face) { 5,  6, 14}, (face) { 6, 15, 14}
	};	
	for(int i = 0; i < numFaces; i++){
		std::cout << i << " = {" << faces[i].a << ", " << faces[i].b << ", " << faces[i].c << "}" <<std::endl;
	}
	
	std::cout << std::endl << "Finding 1 ring neighbors..." << std::endl;
	std::set<int> neighbors[numVerticies] = {};
	for(int v = 0; v < numVerticies; v++){
		for(int f = 0; f < numFaces; f++){			
			if(faces[f].a == v){
				neighbors[v].insert(faces[f].b);
				neighbors[v].insert(faces[f].c);
			}			
			if(faces[f].b == v){
				neighbors[v].insert(faces[f].a);
				neighbors[v].insert(faces[f].c);
			}			
			if(faces[f].c == v){
				neighbors[v].insert(faces[f].a);
				neighbors[v].insert(faces[f].b);
			}
		}	
	}	
	for(int i = 0; i < numVerticies; i++){
		std::cout << i << " meets ";
		for(int j : neighbors[i]){
			std::cout << j << ", ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << "Start Smoothing..." << std::endl;
	for(int i = 0; i < numVerticies; i++){
		std::cout << "Getting smallest edge length (sel) within the 1-ring around the vertex " << i << "..." << std::endl;
		float sel = FLT_MAX; 
		int sel_v = -1;
		for(int n : neighbors[i]){
			float euclidean_norm = sqrt((verticies[i].x - verticies[n].x)*(verticies[i].x - verticies[n].x)
									  + (verticies[i].y - verticies[n].y)*(verticies[i].y - verticies[n].y)
									  + (verticies[i].z - verticies[n].z)*(verticies[i].z - verticies[n].z));
			if(euclidean_norm <= sel){
				sel = euclidean_norm;
				sel_v = n;
			}
		}
		std::cout << "   sel " << sel << " sel_v " << sel_v << std::endl;
	}
}
