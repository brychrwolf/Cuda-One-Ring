#include <fstream>
#include <string>
#include <random>
#include <iostream>
#include "delaunator-cpp-master/include/delaunator.hpp"

const double PI = std::atan(1)*4.;
const int SEED = 1;

void generateVertexCoords(double rand0, double rand1, int radius, double &x, double &y){
	//from https://programming.guide/random-point-within-circle.html
	//To generate a random point within a circle uniformly
	double angle = rand0 * 2 * PI;
	double length = radius * sqrt(rand1);
	x = length * cos(angle);
	y = length * sin(angle);
	//std::cerr << "angle length x y: " << angle << " " << length << " " << x << " " << y << " " << std::endl;
	//std::cerr << "rand0 angle x y: " << rand0 << " " << angle << " " << x << " " << y << " " << std::endl;
}

int main(int ac, char** av){
	int radius = (ac > 1) ? std::stoi(av[1]) : 1e3;
	long numVertices = (ac > 1) ? std::stoi(av[2]) : 1e3;

	// put first vertex in center
	//std::cerr << "put first vertex in center" << std::endl;
	std::vector<double> coords;
	coords.push_back(0);
	coords.push_back(0);
	// generate numVertices-1 many random points in a circle of radius
	//std::cerr << "generate numVertices-1 many random points in a circle of radius" << std::endl;
	double x, y;
	std::minstd_rand gen(SEED);
	std::uniform_real_distribution<> dis(0.0, 1.0);
	for(long i = 0; i < numVertices - 1; i++){
		generateVertexCoords(dis(gen), dis(gen), radius, x, y);
		coords.push_back(x);
		coords.push_back(y);
		//if(i % (numVertices / 10) == 0 )
			//std::cerr << "\t" << i << " of " << numVertices << std::endl;
	}

	//std::cerr << "starting triangulation" << std::endl;
	delaunator::Delaunator d(coords);
	//std::cerr << "finished triangulation" << std::endl;
	long numFaces = d.triangles.size() / 3;
	
	//std::cerr << "initializing ply file" << std::endl;
	std::string ply_fileName = "../random_circle_tesselation_Dirac_delta_"+std::to_string(radius)+"_v"+std::to_string(numVertices)+"_f"+std::to_string(numFaces)+".ply";
	std::ofstream ply_outfile(ply_fileName);
	
	ply_outfile << "ply" << std::endl
			<< "format ascii 1.0" << std::endl
			<< "comment A synthetic circle, subdivided by triangles, whose vertexes are randomly generated and connected via Delauney Triangulation, with a Dirac delta function applied." << std::endl
			<< "comment made by Bryan Wolfford" << std::endl
			<< "element vertex " << numVertices << std::endl
			<< "property float x" << std::endl
			<< "property float y" << std::endl
			<< "property float z" << std::endl
			<< "property float quality" << std::endl
			<< "element face " << numFaces << std::endl
			<< "property list uchar int32 vertex_indices" << std::endl
			<< "end_header" << std::endl;

	//std::cerr << "ply file initialized" << std::endl;
	
	ply_outfile << "0 0 0 1" << std::endl;

	//std::cerr << "writing vertices" << std::endl;
	for(long i = 1; i < numVertices; i++){
		ply_outfile << coords[i*2] << " " << coords[i*2+1] << " 0 0" << std::endl;
	}
	//std::cerr << "finished writing vertices" << std::endl;
	
	//std::cerr << "writing faces" << std::endl;
	for(int i = 0; i < d.triangles.size(); i += 3){
		//d.triangles saves triplets in CW order... print last first for CCW
		ply_outfile << "3 " << d.triangles[i+2] << " " << d.triangles[i+1] << " " << d.triangles[i] << std::endl;
	}
	//std::cerr << "finished writing faces" << std::endl;

	ply_outfile.close();
}
