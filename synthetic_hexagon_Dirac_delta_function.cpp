#include <fstream>
#include <string>

int main(int ac, char** av){
	//int radius = 4;//;1e2;//1e5;
	//if(ac > 1) radius = std::stoi(av[1]);
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
			
	/*for(long i = -1*radius; i < radius+1; i++)
		for(long j = -1*radius; j < radius+1; j++)
		
			int center[2] = {0, 0};
			ply_outfile << center[0] << " " << center[1] << " 0 " << (center[0]==0&&center[1]==0?1:0) << std::endl;
		*/	
	ply_outfile.close();
}

void getPointyHexCorner(double* center, int size, int i, double* point){
	int angle_deg = 60 * i - 30;
	double angle_rad = PI / 180 * angle_deg;
	//return center.x + size * cos(angle_rad),
	//		center.y + size * sin(angle_rad))
}
