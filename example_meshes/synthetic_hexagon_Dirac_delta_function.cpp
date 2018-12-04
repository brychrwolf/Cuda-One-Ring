#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <iostream>
#include <array>
#include <vector>

const double ZERO_TOL = 1e-8;
const double PI = 4*std::atan(1);
int globalVertex = 1;

int calculateNumHexes(int numRings){
	int nthTriNum = numRings*(numRings+1)/2; //1 3 6 10 15; the nth Triangular Number
	return 1 + 6*nthTriNum;
}

int calculateNumFaces(int numRings){
	return 6*calculateNumHexes(numRings);
}

int calculateNumVertices(int numRings){
	//start with a vertex as center of each hex
	int numVertices = calculateNumHexes(numRings);
	//add corners on frontier of each ring
	for(int sideLength = 0; sideLength <= numRings; sideLength++)
		numVertices += 6*(2*sideLength+1);
	return numVertices;
}

void getPoint(int angle, double center[], double distance, double point[]){
	double radians = angle*PI/180;
	point[0] = center[0] + (distance * std::cos(radians));
	point[1] = center[1] + (distance * std::sin(radians));
	
	//zero out values near zero
	for(int i=0;  i<2; i++)
		if(std::abs(point[i]) < ZERO_TOL)
			point[i] = 0;
}

void writePoint(std::ofstream& ply_outfile, double* point){
	ply_outfile << point[0] << " " << point[1] << " 0 0" << std::endl;
}

void writeTriangles(std::ofstream& ply_outfile, double center, double p1, double p2, double p3, double p4, double p5, double p6){
	ply_outfile << "3 " << center << " " << p1 << " " << p2 << std::endl;
	ply_outfile << "3 " << center << " " << p2 << " " << p3 << std::endl;
	ply_outfile << "3 " << center << " " << p3 << " " << p4 << std::endl;
	ply_outfile << "3 " << center << " " << p4 << " " << p5 << std::endl;
	ply_outfile << "3 " << center << " " << p5 << " " << p6 << std::endl;
	ply_outfile << "3 " << center << " " << p6 << " " << p1 << std::endl;
}

int main(int ac, char** av){
	int numRings = 2;
	if(ac > 1) numRings = std::stoi(av[1]);
	int cornerDist = 1;
	double centerDist = std::sqrt(3) * cornerDist;

	int numHexes = calculateNumHexes(numRings);
	int numVertices = calculateNumVertices(numRings);
	int numFaces = calculateNumFaces(numRings);

	int ringAngles[6] = {120, 180, 240, 300, 0, 60};
	int cornerAngles[8] = {30, 90, 150, 210, 270, 330, 30, 90}; //covers wrap around for sides 5,6 

	double cursor[2];
	double center[2];
	double corner[2];
	
	int hexIdx = 0;
	int vertexIdx = 0;
	std::vector<int> centers;

	std::string ply_fileName = "synthetic_hexagon_Dirac_delta_function_"+std::to_string(numRings)+".ply";
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

	//Write first center vertex
	center[0] = 0; center[1] = 0;
	centers.push_back(vertexIdx++);
	ply_outfile << "0 0 0 1" << std::endl;

	//Write corners of first hexagon
	for(int i = 0; i < 6; i++){
		getPoint(cornerAngles[i], center, cornerDist, corner);
		writePoint(ply_outfile, corner);
		vertexIdx++; //Hex corners are not centers
	}
	
	//Write each ring
	for(int ring = 1; ring <= numRings; ring++){		
		//Start just right of center
		cursor[0] = ring*centerDist; cursor[1] = 0;
		for(int sideOfRing = 0; sideOfRing < 6; sideOfRing++){
			for(int hexOnSide = 0; hexOnSide < ring; hexOnSide++){
				//Write the next hex in the direction of this side
				getPoint(ringAngles[sideOfRing], cursor, centerDist, center);
				writePoint(ply_outfile, center);
				centers.push_back(vertexIdx++);
				//Update cursor
				cursor[0] = center[0]; cursor[1] = center[1];
				//write corners (all hexes get two, corner hexes an extra 1)
				for(int ci = 0; ci < (hexOnSide<ring-1?2:3); ci++){
					getPoint(cornerAngles[sideOfRing+ci], cursor, cornerDist, corner);
					writePoint(ply_outfile, corner);
					vertexIdx++; //Hex corners are not centers
				}
			}
		}
	}

	//Triangles
	int numHexesOnRing;
	//Write triangles of middle hex
	int prev_maxVertex = 0;
	int prev_maxCenter = 0;
	int prev_minCenter = 0;
	int maxVertex = 6;
	int maxCenter = 0;
	int minCenter = 0;

	bool prev_isOnCorner = false;
	bool isOnCorner = false;
	int prev_hexCenter = 0;
	int hexCenter = 0;

	int n1, n2, n3, n4;
	int p1 = 1, p2 = 2, p3 = 3, p4 = 4, p5 = 5, p6 = 6;
	writeTriangles(ply_outfile, hexCenter, p1, p2, p3, p4, p5, p6);

	//Write triangles for each hex in each side of each ring
	for(int ring = 1; ring <= numRings; ring++){
		numHexesOnRing = 6*ring;
		prev_maxVertex = maxVertex;
		prev_maxCenter = maxCenter;
		prev_minCenter = minCenter;
		maxVertex = calculateNumVertices(ring)-1;
		maxCenter = maxVertex-3;
		minCenter = prev_maxVertex+1;
		for(int hexIdx = 0; hexIdx < numHexesOnRing; hexIdx++){
			prev_isOnCorner = isOnCorner;
			isOnCorner = ((ring == 1) || ((hexIdx+1) % ring) == 0);
			prev_hexCenter = hexCenter;
			if(prev_isOnCorner){
				hexCenter = ((hexIdx == 0) ? (prev_maxVertex + 1) : (prev_hexCenter + 4));
				n1 = 0;
				n2 = 0;
				n3 = 0;
				n4 = 0;
				p1 = hexCenter+1;
				p2 = hexCenter+2;
				p3 = hexCenter+3;
				p4 = 4;
				p5 = 5;
				p6 = 6;
			}else{
				hexCenter = ((hexIdx == 0) ? (prev_maxVertex + 1) : (prev_hexCenter + 3));
				n1 = 0;
				n2 = 0;
				n3 = 0;
				n4 = 0;
				p1 = hexCenter+1;
				p2 = hexCenter+2;
				p3 = 3;
				p4 = 4;
				p5 = 5;
				p6 = 6;		
			}
			std::cerr << "ring " << ring << " hexIdx " << hexIdx << " isOnCorner " << isOnCorner << " hexCenter " << hexCenter << std::endl;
		}
		std::cerr << std::endl;
	}
	ply_outfile.close();
}
