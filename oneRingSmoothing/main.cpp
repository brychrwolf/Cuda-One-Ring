#include <iostream>
#include <set>
#include <cmath>
#include <cfloat>
#include <random>
#include <array>
#include <vector>
#include <map>

struct vertex {
	float x, y, z;
	
	vertex& operator*(const float scale)
    {
        x = x*scale;
        y = y*scale;
        z = z*scale;
        return *this;
    }
};

typedef int face[3];

float l2norm_diff(vertex pi, vertex p0){
	return sqrt((pi.x - p0.x)*(pi.x - p0.x)
			  + (pi.y - p0.y)*(pi.y - p0.y)
			  + (pi.z - p0.z)*(pi.z - p0.z));
}

int main(){
	/*********************************************************/
	std::cout << std::endl << "  Begin Loading Mesh." << std::endl;
	/*********************************************************/	
	std::cout << "Loading Vertices..." << std::endl;
	const int numVertices = 22;
	vertex vertices[numVertices] = {
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
		-0.373397,  0.645161,
		 0.797587, -0.520541,
		-0.114591,  0.788363,
		-0.936573, -0.699675,
		-0.139383,  0.152594,
		-0.976301,  0.288434,
		-0.212369,  0.722184,
		 0.154177,  0.510287,
		 0.725236,  0.992415,
		 0.582556,  0.272700,
		-0.692900,  0.405410
	};	
	for(int i = 0; i < numVertices; i++){
		std::cout << "vertices[" << i << "] = " << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << " featureVector " << featureVectors[i]<< std::endl;
	}
	
	std::cout << std::endl << "Loading Faces..." << std::endl;
	const int numFaces = 36;
	face faces[numFaces] = {
		{ 0,  1,  8}, { 1, 16,  8},
		{ 1, 12, 16}, {12, 13, 16},
		{13, 17, 16}, { 9, 17, 13},
		{ 2,  9, 13}, { 2,  3,  9},
		{ 3, 10,  9}, { 3,  4, 10},
		{ 4,  5, 10}, { 5, 17, 10},
		{ 5, 14, 17}, {14, 15, 17},
		{15, 16, 17}, {11, 16, 15},
		{ 6, 11, 15}, { 6,  7, 11},
		{ 7,  8, 11}, { 0,  8,  7},
		{ 0, 18,  1}, { 1, 18, 19},
		{ 1, 19,  2}, { 2, 19,  3},
		{ 3, 19,  4}, { 4, 19, 20},
		{ 4, 20,  5}, { 5, 20, 21},
		{ 5, 21,  6}, { 6, 21,  7},
		{ 0,  7, 21}, { 0, 21, 18},
		{ 1,  2, 12}, { 2, 13, 12},
		{ 5,  6, 14}, { 6, 15, 14}
	};	
	for(int i = 0; i < numFaces; i++){
		std::cout << i << " = {" << faces[i][0] << ", " << faces[i][1] << ", " << faces[i][2] << "}" <<std::endl;
	}
	/*********************************************************/
	std::cout << std::endl << "  Finished Loading." << std::endl;
	/*********************************************************/
	/*********************************************************/
	std::cout << std::endl << "  Begin Building Tables..." << std::endl;
	/*********************************************************/
	
	std::cout << std::endl << "Calculating reverse vertex-face lookup table..." << std::endl;
	std::set<int> facesOfVertices[numVertices] = {};
	for(int v = 0; v < 1 /*numVertices*/; v++){
		for(int f = 0; f < numFaces; f++){			
			if(faces[f][0] == v || faces[f][1] == v || faces[f][2] == v ){
				facesOfVertices[v].insert(f);
			}
		}
	}
	for(int v = 0; v < 1 /*numVertices*/; v++){
		std::cout << v << " is a corner of faces: ";
		for(int f : facesOfVertices[v]){
			std::cout << f << ", ";
		}
		std::cout << std::endl;
	}
	
	std::cout << std::endl << "Finding adjacent vertices..." << std::endl;
	std::set<int> adjacentVertices[numVertices] = {};
	for(int v = 0; v < 1 /*numVertices*/; v++){
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
	for(int i = 0; i < 1 /*numVertices*/; i++){
		std::cout << i << " meets ";
		for(int j : adjacentVertices[i]){
			std::cout << j << ", ";
		}
		std::cout << std::endl;
	}
	/*********************************************************/
	std::cout << std::endl << "  Finished Building Tables." << std::endl;
	/*********************************************************/
	/*********************************************************/
	std::cout << std::endl << "  Begin Calculating..." << std::endl;
	/*********************************************************/
	float minEdgeLength[numVertices];
	std::fill_n(minEdgeLength, numVertices, FLT_MAX); //initialize array to max float value
	std::array<std::map<int, float>, numVertices> f_primes;
	std::array<std::map<int, float>, numVertices> f_triangles;
	std::array<std::map<int, float>, numVertices> a_triangles_pythag;
	std::array<std::map<int, float>, numVertices> a_triangles_coord;
		
	std::cout << std::endl << "Iterating over each vertex as p0..." << std::endl;
	for(int p0 = 0; p0 < 1/*numVertices*/; p0++){
	
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
			
			float f_triangle = (featureVectors[p0] + f_primes[p0][pi] + f_primes[p0][pip1]);
			f_triangles[p0].insert(std::pair<int, float>(ti, f_triangle));
			std::cout << "f_triangles[" << p0 << "][" << ti << "] " << f_triangles[p0][ti] << std::endl;
		}
		
		std::cout << std::endl << "Calculating a_triangles_pythag, area to be used as weights..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi = -1;
			int pip1 = -1;
			bool isPiAssigned = false;
			//std::cout << std::endl;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				//std::cout << "v " << v << " ";
				if(v != p0){ // exclude p0
					if(isPiAssigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAssigned = true;
					}
				}
			}
			//std::cout << std::endl;
			//std::cout << "p0 " << p0 << " pi " << pi << " pip1 " << pip1 << " ti " << ti << std::endl;
			
			float scale_pi = minEdgeLength[p0] / l2norm_diff(vertices[pi], vertices[p0]);
			float scale_pip1 = minEdgeLength[p0] / l2norm_diff(vertices[pip1], vertices[p0]);			
			//std::cout << "l2norm_diff(vertices[pi], vertices[p0]) " << l2norm_diff(vertices[pi], vertices[p0]) << " l2norm_diff(vertices[pip1], vertices[p0]) " << l2norm_diff(vertices[pip1], vertices[p0]) << std::endl;		
			//std::cout << "scale_pi " << scale_pi << " scale_pip1 " << scale_pip1 << std::endl;
			
			float b = l2norm_diff(vertices[pip1]*scale_pip1, vertices[pi]*scale_pi);
			float a_triangle = b/2 * sqrt(4*minEdgeLength[p0]*minEdgeLength[p0] - b*b);	
			//std::cout << "b " << b << " a_triangle " << a_triangle << std::endl;
			
			a_triangles_pythag[p0].insert(std::pair<int, float>(ti, a_triangle));
			std::cout << "a_triangles_pythag[" << p0 << "][" << ti << "] " << a_triangles_pythag[p0][ti] << std::endl;
		}
		
		std::cout << std::endl << "Calculating a_triangles_coord, area to be used as weights..." << std::endl;
		std::cout << "Iterating over each facesOfVertices as ti..." << std::endl;		
		for(std::set<int>::iterator ti_iter = facesOfVertices[p0].begin(); ti_iter != facesOfVertices[p0].end(); ti_iter++){
			int ti = *ti_iter;
			
			int pi = -1;
			int pip1 = -1;
			bool isPiAssigned = false;
			//std::cout << std::endl;
			for(int v : faces[ti]){ // for each vertex in this face (a, b, c)
				//std::cout << "v " << v << " ";
				if(v != p0){ // exclude p0
					if(isPiAssigned){
						pip1 = v; // assign the other corner to pip1
					}else{
						pi = v; // assign the first corner to pi
						isPiAssigned = true;
					}
				}
			}
			
			float scale_pi = minEdgeLength[p0] / l2norm_diff(vertices[pi], vertices[p0]);
			float scale_pip1 = minEdgeLength[p0] / l2norm_diff(vertices[pip1], vertices[p0]);			

			vertex v_p0 = vertices[p0];
			vertex v_pi = vertices[pi]*scale_pi;
			vertex v_pip1 = vertices[pip1]*scale_pip1;
			float a = v_p0.x*(v_pi.y-v_pip1.y);
			float b = v_pi.x*(v_pip1.y-v_p0.y);
			float c = v_pip1.x*(v_p0.y-v_pi.y);
			float a_triangle = fabs((a + b + c) / 2);
			
			a_triangles_coord[p0].insert(std::pair<int, float>(ti, a_triangle));
			std::cout << "a_triangles_coord[" << p0 << "][" << ti << "] " << a_triangles_coord[p0][ti] << std::endl;
		}
	}	
	
	float featureVectors_updated[numVertices] = {};
	
}
