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

float l2norm_diff(vertex* vs, int pi, int p0){
	return sqrt((vs[pi].x - vs[p0].x)*(vs[pi].x - vs[p0].x)
			  + (vs[pi].y - vs[p0].y)*(vs[pi].y - vs[p0].y)
			  + (vs[pi].z - vs[p0].z)*(vs[pi].z - vs[p0].z));
}

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
	
	/*std::cout << std::endl << "Generating Random Feature Vectors..." << std::endl;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);
	float featureVectors[numVerticies] = {};
	for(int i = 0; i < numVerticies; i++){
		featureVectors[i] = dis(gen);
		std::cout << "featureVector [" << i << "] = " << featureVectors[i] << std::endl;
	}*/
	
	std::cout << std::endl << "Set Feature Vectors..." << std::endl;
	float featureVectors[numVerticies] = {	
		-0.373397,
		 0.645161,
		 0.797587,
		-0.520541,
		-0.114591,
		 0.788363,
		-0.936573,
		-0.699675,
		-0.139383,
		 0.152594,
		-0.976301,
		 0.288434,
		-0.212369,
		 0.722184,
		 0.154177,
		 0.510287,
		 0.725236,
		 0.992415,
		 0.582556,
		 0.2727,
		-0.6929,
		 0.40541
	};
	
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
	for(int p0 = 0; p0 < numVerticies; p0++){
		int sel_v = -1;
		for(std::set<int>::iterator pi_iter = neighbors[p0].begin(); pi_iter != neighbors[p0].end(); pi_iter++){
			int pi = *pi_iter;
			//IDEA: These norms are used later, so can save calculations if values are stored.
			float norm_diff = l2norm_diff(verticies, pi, p0);
			if(norm_diff <= sel[p0]){
				sel[p0] = norm_diff;
				sel_v = pi;
			}
			std::cout  << "p0 " << p0  << " pi " << pi << " norm_diff " << norm_diff << std::endl;
		}
		std::cout << "sel[" << p0 << "] " << sel[p0] << " sel_v " << sel_v << std::endl;
	}
	
	std::cout << std::endl << "Calculating f', weighted mean f0 and fi by distance..." << std::endl;
	std::array<std::map<int, float>, numVerticies> f_primes;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		for(int pi : neighbors[p0]){
			//IDEA: Saving to new file, too slow. Saving to new whole array, too large.
			//Only if I know if a vertex will never be processed again, can I save updated values back into the original memspace.
			//That knowledge comes from 2ring neighbors. Does saving that Info actually save memory? 
			//n^3 (or worse) complexity is too sloow also! Oh... but can derive 2-ring from the 1ring neighbors list (which is MUCH faster (is it?))
			float f_prime = featureVectors[p0] + sel[p0] * (featureVectors[pi] - featureVectors[p0]) / l2norm_diff(verticies, pi, p0);
			f_primes[p0].insert(std::pair<int, float>(pi, f_prime));
			std::cout << "f_primes[" << p0 << "][" << pi << "] " << f_primes[p0][pi] << std::endl;
		}
	}

	std::cout << std::endl << "Calculating f^t/3, weighted mean of function value at triangles..." << std::endl;
	std::array<std::map<int, float>, numVerticies> f_triangles;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		std::set<int>::iterator b = neighbors[p0].begin();
		std::set<int>::iterator e = neighbors[p0].end();
		for(std::set<int>::iterator pi_iter = b; pi_iter != e; pi_iter++){
			std::set<int>::iterator pip1_iter = (std::next(pi_iter, 1) != e) ? std::next(pi_iter, 1) : b; // wrap around indexing //TODO: how can one ensure indexs are in order, and adjacent endges have index +/1?!
			int pi = *pi_iter;
			int pip1 = *pip1_iter;
			float f_triangle = (featureVectors[p0] + f_primes[p0][pi] + f_primes[p0][pip1])/3;
			f_triangles[p0].insert(std::pair<int, float>(pi, f_triangle));
			std::cout << "f_trianlges[" << p0 << "][" << pi << "] (pip1=" << pip1 << ") " << f_triangles[p0][pi] << std::endl;
		}
	}

	/*std::cout << std::endl << "Calculating areas of all triangles in Geodesic Disks ..." << std::endl;
	std::array<std::map<int, float>, numVerticies> areas_of_triangles;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		for(int pi = 1; pi < neighbors[p0].size(); pi++){ // intentionally skip point zero, as its part of all triangles
			int pip1 = (pi+1) % neighbors[p0].size(); // wrap around indexing //TODO: how can one ensure indexs are in order, and adjacent endges have index +/1?!
			float l2n_d = l2norm_diff(verticies, pi, pip1); //TODO: verticies does not contain correct values for pi or pip1... they must be scaled by sel[p0]
			std::cout << "4*sel[p0]^2 " << 4*sel[p0]*sel[p0] << " l2n_d*l2n_d " << l2n_d*l2n_d << std::endl;
			float area_of_triangle = l2n_d/4 * sqrt(4*sel[p0]*sel[p0] - l2n_d*l2n_d);
			areas_of_triangles[p0].insert(std::pair<int, float>(pi, area_of_triangle));
			std::cout << "areas_of_triangles[" << p0 << "][" << pi << "] " << areas_of_triangles[p0][pi] << std::endl;
		}
	}

	std::cout << std::endl << "Calculating weights from angles in Geodesic Disks ..." << std::endl;
	std::array<std::map<int, float>, numVerticies> angle_weights;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		for(int pi = 1; pi < neighbors[p0].size(); pi++){ // intentionally skip point zero, as its part of all triangles
			int pip1 = (pi+1) % neighbors[p0].size(); // wrap around indexing //TODO: how can one ensure indexs are in order, and adjacent endges have index +/1?!
			float alpha = XXXX;
			float angle_weight = sin(alpha)sel[p0]*sel[p0];
			angle_weights[p0].insert(std::pair<int, float>(pi, angle_weight));
			std::cout << "angle_weights[" << p0 << "][" << pi << "] " << angle_weights[p0][pi] << std::endl;
		}
	}
	
	std::cout << std::endl << "Calculating weighted mean function valuewith total area ..." << std::endl;
	std::array<float, numVerticies> weighted_mean_function_values;	
	for(int p0 = 0; p0 < numVerticies; p0++){
		for(int pi = 1; pi < neighbors[p0].size(); pi++){ // intentionally skip point zero, as its part of all triangles
			int pip1 = (pi+1) % neighbors[p0].size(); // wrap around indexing //TODO: how can one ensure indexs are in order, and adjacent endges have index +/1?!

			float alpha = XXXX;
			float weighted_mean_function_value = sin(alpha)sel[p0]*sel[p0];
		}
		angle_weights[p0].insert(std::pair<int, float>(pi, angle_weight));
		std::cout << "angle_weights[" << p0 << "][" << pi << "] " << angle_weights[p0][pi] << std::endl;
	}*/
	
	
	float featureVectors_updated[numVerticies] = {};
	
}
