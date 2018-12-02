#include <fstream>
#include <string>
#include <cmath>

#define PI 4*std::atan(1);

int main(int ac, char** av){
	int radius = 1;//;1e2;//1e5;
	if(ac > 1) radius = std::stoi(av[1]);
	int size = 1;
	//int diameter = 2*radius + 1;
	//int numVertices = 4*radius*radius + 4*radius + 1;
	//int numFaces = 2*(diameter-1)*(diameter-1);

	std::string ply_fileName = "example_meshes/synthetic_hexagon_Dirac_delta_function_"+std::to_string(radius)+".ply";
	std::ofstream ply_outfile(ply_fileName);
	
	ply_outfile << "ply" << std::endl
			<< "format ascii 1.0" << std::endl
			<< "comment A synthetic hexagon, subdivided by triangles, with a Dirac delta function applied." << std::endl
			<< "comment made by Bryan Wolfford" << std::endl
			//<< "element vertex " << numVertices << std::endl
			<< "property int x" << std::endl
			<< "property int y" << std::endl
			<< "property int z" << std::endl
			<< "property float quality" << std::endl
			//<< "element face " << numFaces << std::endl
			<< "property list uchar int32 vertex_indices" << std::endl
			<< "end_header" << std::endl;
//r	0, 1,  2,  3
//h	1, 6, 12, 18
	double* center[2] = {0, 0};
	ply_outfile << "0 0 0 1" << std::endl;
	for(int angle = 30; angle < 360; angle += 60){
		int radian = angle*PI/180;
		double x = center[0] + size * std::cos(radian);
		double y = center[1] + size * std::sin(radian);
		ply_outfile << x << " " << y << " 0 0" << std::endl;
	}
	//for(long r = 0; r < radius; r++)
	//	ply_outfile << x << " " << y << " 0 " << (x==0&&y==0?1:0) << std::endl;
	ply_outfile.close();
}
