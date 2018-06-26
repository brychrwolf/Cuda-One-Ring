#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>
#include <array>
#include <vector>
#include <map>

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
	const int numVerticies = 22;
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
	const int numFaces = 36;
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
	
	std::cout << std::endl << "Getting smallest edge length (sel) within the 1-ring around all verticies." << std::endl;
	float sel[numVerticies];
	std::fill_n(sel, numVerticies, FLT_MAX); //initialize array to max float value
	for(int i = 0; i < numVerticies; i++){
		int sel_v = -1;
		for(int j : neighbors[i]){
			//IDEA: These norms are used later, so can save calculations if values are store.
			float euclidean_norm = sqrt((verticies[j].x - verticies[i].x)*(verticies[j].x - verticies[i].x)
									  + (verticies[j].y - verticies[i].y)*(verticies[j].y - verticies[i].y)
									  + (verticies[j].z - verticies[i].z)*(verticies[j].z - verticies[i].z));
			if(euclidean_norm <= sel[i]){
				sel[i] = euclidean_norm;
				sel_v = j;
			}
		}
		std::cout << "sel[" << i << "] " << sel[i] << " sel_v " << sel_v << std::endl;
	}
	
	std::cout << std::endl << "Calculating all f`_i..." << std::endl;	
	
	std::cout << std::endl << "Calculating all weights..." << std::endl;
	std::array<std::map<int, float>, numVerticies> weights;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		for(int pi : neighbors[p0]){
			//IDEA: Saving to new file, too slow. Saving to new whole array, too large.
			//Only if I know if a vertex will never be processed again, can I save updated values back into the original memspace.
			//That knowledge comes from 2ring neighbors. Does saving that Info actually save memory? 
			//n^3 (or worse) complexity is too sloow also! Oh... but can derive 2-ring from the 1ring neighbors list (which is MUCH faster)
			float weight = featureVectors[p0] + sel[p0] * (featureVectors[pi] - featureVectors[p0])
					/ sqrt((verticies[pi].x - verticies[p0].x)*(verticies[pi].x - verticies[p0].x)
						 + (verticies[pi].y - verticies[p0].y)*(verticies[pi].y - verticies[p0].y)
						 + (verticies[pi].z - verticies[p0].z)*(verticies[pi].z - verticies[p0].z));
			weights[p0].insert(std::pair<int, float>(pi, weight));
			std::cout << "weights[" << p0 << "][" << pi << "] " << weights[p0][pi] << std::endl;
		}
	}
	
	float featureVectors_updated[numVerticies] = {};
	
}
