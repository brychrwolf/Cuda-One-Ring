#include <fstream>
#include <string>

int main(int ac, char** av){
	int radius = 4;//;1e2;//1e5;
	if(ac > 1) radius = std::stoi(av[1]);
	int diameter = 2*radius + 1;
	long numVertices_frame = 4*radius*radius + 4*radius + 1;
	long numVertices_centers = 4*radius*radius;
	long numVertices = numVertices_frame + numVertices_centers;
	long numFaces = 4*(diameter-1)*(diameter-1);

	std::string ply_fileName = "../square_tesselation_4tri_Dirac_delta_"+std::to_string(radius)+"_v"+std::to_string(numVertices)+"_f"+std::to_string(numFaces)+".ply";
	std::ofstream ply_outfile(ply_fileName);
	
	ply_outfile << "ply" << std::endl
			<< "format ascii 1.0" << std::endl
			<< "comment A synthetic square, subdivided by triangles, with a Dirac delta function applied." << std::endl
			<< "comment made by Bryan Wolfford" << std::endl
			<< "element vertex " << numVertices << std::endl
			<< "property float x" << std::endl
			<< "property float y" << std::endl
			<< "property float z" << std::endl
			<< "property float quality" << std::endl
			<< "element face " << numFaces << std::endl
			<< "property list uchar int32 vertex_indices" << std::endl
			<< "end_header" << std::endl;

	//write frame vertices (each square is 2x2)
	for(long i = -2*radius; i < 2*radius+1; i+=2)
		for(long j = -2*radius; j < 2*radius+1; j+=2)
			ply_outfile << i << " " << j << " 0 " << (i==0&&j==0?1:0) << std::endl;
	//write middle vertices
	for(long i = -2*radius+1; i < 2*radius; i+=2)
		for(long j = -2*radius+1; j < 2*radius+1; j+=2)
			ply_outfile << i << " " << j << " 0 " << (i==0&&j==0?1:0) << std::endl;
	//write faces
	long row = 0;
	for(long i = 0; i < numVertices_centers; i++){
		if(i > 0 && i % (2*radius) == 0) row++;
		long center = i+numVertices_frame;
		long a = i+row;
		long b = a+1, c=a+diameter, d=c+1;
		ply_outfile << "3 " << center << " " << a << " " << c << std::endl;
		ply_outfile << "3 " << center << " " << c << " " << d << std::endl;
		ply_outfile << "3 " << center << " " << d << " " << b << std::endl;
		ply_outfile << "3 " << center << " " << b << " " << a << std::endl;
	}
	ply_outfile.close();
}
