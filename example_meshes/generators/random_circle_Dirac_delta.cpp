#include <fstream>
#include <string>
#include <random>
#include <iostream>

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
	std::cerr << "rand0 angle x y: " << rand0 << " " << angle << " " << x << " " << y << " " << std::endl;
}

int main(int ac, char** av){
	int radius = (ac > 1) ? std::stoi(av[1]) : 1e3;
	int numVertices = (ac > 1) ? std::stoi(av[2]) : 1e3;

	double x, y;
	std::minstd_rand gen(SEED);
	std::uniform_real_distribution<> dis(0.0, 1.0);
	
	int numFaces = 0; //TODO delaney triangulation 

	std::string ply_fileName = "../random_circle_tesselation_Dirac_delta_"+std::to_string(radius)+"_"+std::to_string(numVertices)+".ply";
	std::ofstream ply_outfile(ply_fileName);
	
	ply_outfile << "ply" << std::endl
			<< "format ascii 1.0" << std::endl
			<< "comment A synthetic circle, subdivided by triangles, whose vertexes are randomly generated, with a Dirac delta function applied." << std::endl
			<< "comment made by Bryan Wolfford" << std::endl
			<< "element vertex " << numVertices << std::endl
			<< "property int x" << std::endl
			<< "property int y" << std::endl
			<< "property int z" << std::endl
			<< "property float quality" << std::endl
			<< "element face " << numFaces << std::endl
			<< "property list uchar int32 vertex_indices" << std::endl
			<< "end_header" << std::endl;
	
	ply_outfile << "0 0 0 1" << std::endl;
	for(int i = 0; i < numVertices - 1; i++){
		generateVertexCoords(dis(gen), dis(gen), radius, x, y);
		ply_outfile << x << " " << y << " 0 0" << std::endl;
	}

	ply_outfile.close();
}
