#include <fstream>
#include <string>
#include <cmath>

int main(int ac, char** av){
	const double PI = 4*std::atan(1);
	int radius = 1;//;1e2;//1e5;
	if(ac > 1) radius = std::stoi(av[1]);
	int size = 1;
	double width = std::sqrt(3) * size;
	double height = 2 * size;
	//int diameter = 2*radius + 1;
	int numVertices = 7+(4*6);
	int numFaces = 6+(6*6);

	std::string ply_fileName = "synthetic_hexagon_Dirac_delta_function_"+std::to_string(radius)+".ply";
	std::ofstream ply_outfile(ply_fileName);

	ply_outfile << "ply" << std::endl
			<< "format ascii 1.0" << std::endl
			<< "comment A synthetic hexagon, subdivided by triangles, with a Dirac delta function applied." << std::endl
			<< "comment made by Bryan Wolfford" << std::endl
			<< "element vertex " << numVertices << std::endl
			<< "property int x" << std::endl
			<< "property int y" << std::endl
			<< "property int z" << std::endl
			<< "property float quality" << std::endl
			<< "element face " << numFaces << std::endl
			<< "property list uchar int32 vertex_indices" << std::endl
			<< "end_header" << std::endl;
//r	0, 1,  2,  3
//h	1, 6, 12, 18
	//Write center
	ply_outfile << "0 0 0 1" << std::endl;
	//Write corners of middle hexagon
	for(int i = 0; i < 6; i++){
		int angle = -30 + i*60;
		double radian = angle*PI/180;
		double x = size * std::cos(radian);
		double y = size * std::sin(radian);
		ply_outfile << x << " " << y << " 0 0" << std::endl;
	}
	//Write centers of first ring
	for(int i = 0; i < 6; i++){
		int angle = i*60;
		double radian = angle*PI/180;
		double x = 0 + (width * std::cos(radian));
		double y = 0 + (width * std::sin(radian));
		ply_outfile << x << " " << y << " 0 0" << std::endl;

		double center[2] = {x, y};
		//Write new corners of first ring		
		for(int j = 0; j < 3; j++){
			int angle = -30 + (j+i)*60;
			double radian = angle*PI/180;
			double x = center[0] + (size * std::cos(radian));
			double y = center[1] + (size * std::sin(radian));
			ply_outfile << x << " " << y << " 0 0" << std::endl;
		}
	}

	////Triangles
	//Write triangles for center hex
	for(int i = 0+1; i < 6+1; i++)
		ply_outfile << "3 0 " << i << " " << (i!=6 ? i+1 : 1) << std::endl;

	//Write triangles for first ring hexes
	int ringMax = 7 + 5*4 + 3;
	for(int i = 0; i < 6; i++){
		int hexCenter = 7 + i*4;
		int prevHexMax = 7 + (i-1)*4 + 3;
		int p1 = hexCenter+1;
		int p2 = hexCenter+2;
		int p3 = hexCenter+3;
		int p4 = i<5 ? i+2 : 0+1;
		int p5 = i+1;
		int p6 = i>0 ? prevHexMax : ringMax;
		ply_outfile << "3 " << hexCenter << " " << p1 << " " << p2 << std::endl;
		ply_outfile << "3 " << hexCenter << " " << p2 << " " << p3 << std::endl;
		ply_outfile << "3 " << hexCenter << " " << p3 << " " << p4 << std::endl;
		ply_outfile << "3 " << hexCenter << " " << p4 << " " << p5 << std::endl;
		ply_outfile << "3 " << hexCenter << " " << p5 << " " << p6 << std::endl;
		ply_outfile << "3 " << hexCenter << " " << p6 << " " << p1 << std::endl;
	} 
	ply_outfile.close();
}
