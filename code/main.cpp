#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>
#include <array>
#include <vector>
#include <map>

int P0_BEGIN;
int P0_END;

typedef std::array<float, 3> vertex;
typedef std::array<int, 3> face;

vertex scale(vertex v, float scalar){
	return {v[0]*scalar,
			v[1]*scalar, 
			v[2]*scalar};
}

vertex combine(vertex v1, vertex v2){
	return {v1[0] + v2[0],
			v1[1] + v2[1],
			v1[2] + v2[2]};
}


float l2norm(const vertex pi){
	return sqrt(pi[0]*pi[0]
			  + pi[1]*pi[1]
			  + pi[2]*pi[2]);
}

float l2norm_diff(const vertex pi, const vertex p0){
	return sqrt((pi[0] - p0[0])*(pi[0] - p0[0])
			  + (pi[1] - p0[1])*(pi[1] - p0[1])
			  + (pi[2] - p0[2])*(pi[2] - p0[2]));
}

int main(){
	/******************************************************************/
	std::cout << std::endl << "****** Begin Loading Mesh." << std::endl;
	/******************************************************************/
	std::cout << "Loading Vertices..." << std::endl;
	const int numVertices = 22;
	vertex vertices[numVertices] = {
		{ 0,  0,  0}, { 2,  0,  0}, //  0,  1
		{12,  0,  0}, {14,  0,  0}, //  2,  3
		{14, 20,  0}, {12, 20,  0}, //  4,  5
		{ 2, 20,  0}, { 0, 20,  0}, //  6,  7
		{ 1,  1, -1}, {13,  1, -1}, //  8,  9
		{13, 19, -1}, { 1, 19, -1}, // 10, 11
		{ 2, 10,  0}, {12, 10,  0}, // 12, 13
		{12, 12,  0}, { 2, 12,  0}, // 14, 15
		{ 1, 11, -1}, {13, 11, -1}, // 16, 17
		{-2, -2,  0}, {16, -2,  0}, // 18, 19
		{16, 22,  0}, {-2, 22,  0}  // 20, 21
	};	
	
	/*std::cout << std::endl << "Generating Random Feature Vectors..." << std::endl;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);
	float featureVectors[numVertices] = {};
	for(int i = 0; i < numVertices; i++){
		featureVectors[i] = dis(gen);
		std::cout << "featureVector [" << i << "] = " << featureVectors[i] << std::endl;
	}*/
	
	std::cout << "Set Feature Vectors..." << std::endl;
	float featureVectors[numVertices] = {	
		-0.373397,  0.645161, //  0,  1
		 0.797587, -0.520541, //  2,  3
		-0.114591,  0.788363, //  4,  5
		-0.936573, -0.699675, //  6,  7
		-0.139383,  0.152594, //  8,  9
		-0.976301,  0.288434, // 10, 11
		-0.212369,  0.722184, // 12, 13
		 0.154177,  0.510287, // 14, 15
		 0.725236,  0.992415, // 16, 17
		 0.582556,  0.272700, // 18, 19
		-0.692900,  0.405410  // 20, 21
	};	
	for(int i = 0; i < numVertices; i++){
		std::cout << "vertices[" << i << "] = " << vertices[i][0] << ", " << vertices[i][1] << ", " << vertices[i][2] << " featureVector " << featureVectors[i]<< std::endl;
	}
	
	std::cout << std::endl << "Loading Faces..." << std::endl;
	const int numFaces = 36;
	face faces[numFaces] = {
		{ 0,  1,  8}, { 1, 16,  8}, //  0,  1
		{ 1, 12, 16}, {12, 13, 16}, //  2,  3
		{13, 17, 16}, { 9, 17, 13}, //  4,  5
		{ 2,  9, 13}, { 2,  3,  9}, //  6,  7
		{ 3, 10,  9}, { 3,  4, 10}, //  8,  9
		{ 4,  5, 10}, { 5, 17, 10}, // 10, 11
		{ 5, 14, 17}, {14, 15, 17}, // 12, 13
		{15, 16, 17}, {11, 16, 15}, // 14, 15
		{ 6, 11, 15}, { 6,  7, 11}, // 16, 17
		{ 7,  8, 11}, { 0,  8,  7}, // 18, 19
		{ 0, 18,  1}, { 1, 18, 19}, // 20, 21
		{ 1, 19,  2}, { 2, 19,  3}, // 22, 23
		{ 3, 19,  4}, { 4, 19, 20}, // 24, 25
		{ 4, 20,  5}, { 5, 20, 21}, // 26, 27
		{ 5, 21,  6}, { 6, 21,  7}, // 28, 29
		{ 0,  7, 21}, { 0, 21, 18}, // 30, 31
		{ 1,  2, 12}, { 2, 13, 12}, // 32, 33
		{ 5,  6, 14}, { 6, 15, 14}  // 34, 35
	};	
	for(int i = 0; i < numFaces; i++){
		std::cout << i << " = {" << faces[i][0] << ", " << faces[i][1] << ", " << faces[i][2] << "}" <<std::endl;
	}
	/******************************************************************/
	std::cout << std::endl << "****** Finished Loading." << std::endl;
	/******************************************************************/
	/******************************************************************/
	std::cout << std::endl << "****** Begin Building Tables..." << std::endl;
	/******************************************************************/
	int P0_BEGIN = 0;
	int P0_END = numVertices;
	
	std::cout << std::endl << "Calculating reverse vertex-face lookup table..." << std::endl;
	std::set<int> facesOfVertices[numVertices] = {};
	for(int v = P0_BEGIN; v < P0_END; v++){
		for(int f = 0; f < numFaces; f++){			
			if(faces[f][0] == v || faces[f][1] == v || faces[f][2] == v ){
				facesOfVertices[v].insert(f);
			}
		}
	}
	for(int v = P0_BEGIN; v < P0_END; v++){
		std::cout << v << " is a corner of faces: ";
		for(int f : facesOfVertices[v]){
			std::cout << f << ", ";
		}
		std::cout << std::endl;
	}
	
	std::cout << std::endl << "Finding adjacent vertices..." << std::endl;
	std::set<int> adjacentVertices[numVertices] = {};
	for(int v = P0_BEGIN; v < P0_END; v++){
		for(int f : facesOfVertices[v]){
			if(faces[f][0] == v){
				adjacentVertices[v].insert(faces[f][1]);
				adjacentVertices[v].insert(faces[f][2]);
			}			
			else if(faces[f][1] == v){
				adjacentVertices[v].insert(faces[f][0]);
				adjacentVertices[v].insert(faces[f][2]);
			}			
			else if(faces[f][2] == v){
				adjacentVertices[v].insert(faces[f][0]);
				adjacentVertices[v].insert(faces[f][1]);
			}
		}	
	}	
	for(int i = P0_BEGIN; i < P0_END; i++){
		std::cout << i << " meets ";
		for(int j : adjacentVertices[i]){
			std::cout << j << ", ";
		}
		std::cout << std::endl;
	}
	/******************************************************************/
	std::cout << std::endl << "****** Finished Building Tables." << std::endl;
	/******************************************************************/
	/******************************************************************/
	std::cout << std::endl << "****** Begin Calculating..." << std::endl;
	/******************************************************************/
	float minEdgeLength[numVertices];
	std::fill_n(minEdgeLength, numVertices, FLT_MAX); // initialize array to max float value
	std::array<std::map<int, float>, numVertices> f_primes; // function value at delta_min along pi
	std::array<std::map<int, float>, numVertices> f_triangles; // function value of triangles 
	std::array<std::map<int, float>, numVertices> a_triangles_pythag; // area of geodesic triangles to be used as weights
	std::array<std::map<int, float>, numVertices> a_triangles_coord; // area of geodesic triangles to be used as weights
	float wa_geoDisks[numVertices] = {}; // weighted area of triangles comprising total geodiseic disk
		
	std::cout << std::endl << "Iterating over each vertex as p0..." << std::endl;
	for(int p0 = P0_BEGIN; p0 < P0_END; p0++){


	
		std::cout << std::endl << "Calculating minimum edge length among adjacent vertices..." << std::endl;
		int minEdgeLength_vertex = -1; // a minimum must exist, error if none is found
		std::cout << "Iterating over each adjacent_vertex as pi..." << std::endl;
		for(std::set<int>::iterator pi_iter = adjacentVertices[p0].begin(); pi_iter != adjacentVertices[p0].end(); pi_iter++){
			int pi = *pi_iter;
			float norm_diff = l2norm_diff(vertices[pi], vertices[p0]); //TODO: used twice, for p0 and when p1 becomes p0. Would saving value make a big difference?
			if(norm_diff <= minEdgeLength[p0]){
				minEdgeLength[p0] = norm_diff;
				minEdgeLength_vertex = pi;
			}
			std::cout  << "p0 " << p0 << " pi " << pi << " norm_diff " << norm_diff << std::endl;
		}
		std::cout << "minEdgeLength[" << p0 << "] " << minEdgeLength[p0] << " minEdgeLength_vertex " << minEdgeLength_vertex << std::endl;


		std::cout << std::endl << "Calculating f', weighted mean f0 and fi by distance..." << std::endl;
		std::cout << "Iterating over each adjacent_vertex as pi..." << std::endl;		
		for(std::set<int>::iterator pi_iter = adjacentVertices[p0].begin(); pi_iter != adjacentVertices[p0].end(); pi_iter++){
			int pi = *pi_iter;
			float f_prime = featureVectors[p0] + minEdgeLength[p0] * (featureVectors[pi] - featureVectors[p0]) / l2norm_diff(vertices[pi], vertices[p0]);
			f_primes[p0].insert(std::pair<int, float>(pi, f_prime));
			std::cout << "f_primes[" << p0 << "][" << pi << "] " << f_primes[p0][pi] << std::endl;
		}

		
		std::cout << std::endl << "Calculating f_triangles, weighted mean (f0 + f'i + f'ip1)/3..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi;
			int pip1;
			bool isPiAsigned = false;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				if(v != p0){ // exclude p0
					if(!isPiAsigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAsigned = true;
					}
				}
			}					
			
			float f_triangle = (featureVectors[p0] + f_primes[p0][pi] + f_primes[p0][pip1]); //save the /3 for later like in paper
			f_triangles[p0].insert(std::pair<int, float>(ti, f_triangle));
			std::cout << "f_triangles[" << p0 << "][" << ti << "] " << f_triangles[p0][ti] << std::endl;
		}


		
		std::cout << std::endl << "Calculating a_triangles_pythag, area to be used as weights..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi, pip1;
			bool isPiAssigned = false;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				if(v != p0){ // exclude p0
					if(isPiAssigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAssigned = true;
					}
				}
			}

			vertex relative_pi = combine(vertices[pi],   scale(vertices[p0], -1));
			vertex unit_pi = scale(relative_pi, 1/l2norm(relative_pi));
			vertex scaled_pi = scale(unit_pi, minEdgeLength[p0]);
			vertex mel_pi = combine(scaled_pi, vertices[p0]);
			
			vertex relative_pip1 = combine(vertices[pip1],   scale(vertices[p0], -1));
			vertex unit_pip1 = scale(relative_pip1, 1/l2norm(relative_pip1));
			vertex scaled_pip1 = scale(unit_pip1, minEdgeLength[p0]);
			vertex mel_pip1 = combine(scaled_pip1, vertices[p0]);
			
			float base = l2norm_diff(mel_pip1, mel_pi);
			float height = sqrt(minEdgeLength[p0]*minEdgeLength[p0] - (base/2)*(base/2));
			float a_triangle = base * height / 2;

			// or like as paper
			//float a_triangle = base/4 * sqrt(4*minEdgeLength[p0]*minEdgeLength[p0] - base*base); // multiplying by 4 inside the sqrt countered by dividing by 2 outside

			a_triangles_pythag[p0].insert(std::pair<int, float>(ti, a_triangle));
			std::cout << "a_triangles_pythag[" << p0 << "][" << ti << "] " << a_triangles_pythag[p0][ti] << std::endl;
		}
		
		

		std::cout << std::endl << "Calculating a_geoDisks, weighted mean function value over total area of adjacent triangles..." << std::endl;
		float area = 0.0;
		float weighted_area = 0.0;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			area += a_triangles_pythag[p0][ti];
			float wa = a_triangles_pythag[p0][ti] * f_triangles[p0][ti];
			weighted_area += wa;
			std::cout << "weighted_area[" << p0 << "]" << "[" << ti << "] = " << wa << std::endl;
		}
		std::cout << "total area " << area << std::endl;
		std::cout << "total weighted_area " << weighted_area << std::endl;
		float wa_geoDisk = weighted_area / (3 * area); // /3 was carried over from from the f_triangles calculations
		wa_geoDisks[p0] = (wa_geoDisk);
		std::cout << "wa_geoDisks[" << p0 << "] " << wa_geoDisks[p0] << std::endl;
	}
	
}
