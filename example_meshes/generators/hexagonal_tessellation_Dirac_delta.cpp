#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <iostream>
#include <array>
#include <vector>

const bool DEBUG = false;//true;
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

	int numVertices = calculateNumVertices(numRings);
	int numFaces = calculateNumFaces(numRings);

	int ringAngles[6] = {120, 180, 240, 300, 0, 60};
	int cornerAngles[8] = {30, 90, 150, 210, 270, 330, 30, 90}; //covers wrap around for sides 5,6 

	double cursor[2];
	double center[2];
	double corner[2];

	int vertexIdx = 0;
	std::vector<int> centers;

	std::string ply_fileName = "../hexagonal_tessellation_Dirac_delta_"+std::to_string(numRings)+".ply";
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

	//Write corners of first hexagon (start with last two angles)
	int offset = 2;
	for(int i = 0+offset; i < 6+offset; i++){
		getPoint(cornerAngles[i%6], center, cornerDist, corner);
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
	
	if(DEBUG) for(size_t i = 0; i < centers.size(); i++)
		std::cerr << "centers[" << i << "] " << centers[i] << std::endl;

	//Triangles
	int numHexesOnRing;
	int globalHexIndex = 0;
	int n2Index = 0;
	//Write triangles of middle hex
	int prev_maxVertex = 0;
	//int prev_maxCenter = 0;
	//int prev_minCenter = 0;
	int maxVertex = 6;
	//int maxCenter = 0;
	//int minCenter = 0;

	bool isOnCorner = false;
	//int prev_hexCenter = 0;
	int hexCenter = 0;

	int prev_n2 = 0;
	int n2 = 0; //int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
	int p1 = 1, p2 = 2, p3 = 3, p4 = 4, p5 = 5, p6 = 6;
	writeTriangles(ply_outfile, hexCenter, p1, p2, p3, p4, p5, p6);
	globalHexIndex++;

	//Write triangles for each hex in each side of each ring
	for(int ring = 1; ring <= numRings; ring++){
		numHexesOnRing = 6*ring;
		prev_maxVertex = maxVertex;
		//prev_maxCenter = maxCenter;
		//prev_minCenter = minCenter;
		maxVertex = calculateNumVertices(ring)-1;
		//maxCenter = maxVertex-3;
		//minCenter = prev_maxVertex+1;
		if(DEBUG){
			std::cerr << "ring " << ring << " numHexesOnRing " << numHexesOnRing << " maxVertex " << maxVertex << std::endl;
			std::cerr << "prev_maxVertex " << prev_maxVertex << std::endl << std::endl;
		}
		for(int hexIdx = 0; hexIdx < numHexesOnRing; hexIdx++){
			isOnCorner = ((ring == 1) || ((hexIdx+1) % ring) == 0);

			prev_n2 = n2;
			//prev_hexCenter = hexCenter;
			hexCenter = centers[globalHexIndex];

			//		n1 = ((hexIdx == numHexesOnRing - 1) ? (minCenter) : (centers[globalHexIndex + 1]));
			if(ring == 1){
					n2 = 0;
			//		n3 = prev_hexCenter;
			//		n4 = maxCenter; //doesn't exist		
					p1 = hexCenter + 1;
					p2 = hexCenter + 2;
					p3 = hexCenter + 3;
					p4 = hexIdx == 0 ? 6 : hexIdx;
					p5 = hexIdx == 1 ? 6 : p4 - 1;
					p6 = hexIdx == 0 ? maxVertex : hexCenter - 1;
			}else if(hexIdx == 0){
					n2 = (ring > 1 ? centers[++n2Index] : 0);
			//		n3 = prev_hexCenter;
			//		n4 = maxCenter; //doesn't exist		
					p1 = hexCenter + 1;
					p2 = hexCenter + 2;
					p3 = (ring > 1 ? n2 + 1 : hexCenter + 3);
					p4 = prev_maxVertex;
					p5 = p4 - 1;
					p6 = maxVertex;
			}else{
				if(isOnCorner){
					n2 = prev_n2;
			//		n3 = prev_hexCenter;
			//		n4 = -1; //doesn't exist					
					p1 = hexCenter + 1;
					p2 = hexCenter + 2;
					p3 = hexCenter + 3;
					p4 = n2 + 2;
					p5 = p4 - 1;
					p6 = hexCenter - 1;
				}else{
					n2 = centers[++n2Index];
			//		n3 = prev_n2;
			//		n4 = prev_hexCenter;
					p1 = hexCenter+1;
					p2 = hexCenter+2;
					p3 = centers[n2Index] + 1;
					p4 = n2 - 1;
					p5 = p4 - 1;
					p6 = hexCenter - 1;		
				}
			}
			if(DEBUG){
				std::cerr << "globalHexIndex " << globalHexIndex << " hexIdx " << hexIdx << " isOnCorner " << isOnCorner << " hexCenter " << hexCenter << std::endl;
				//std::cerr << "   n1 " << n1 << " n2 " << n2 << " n3 " << n3 << " n4 " << n4 << std::endl;
				std::cerr << "   p1 " << p1 << " p2 " << p2 << " p3 " << p3 << " p4 " << p4 << " p5 " << p5 << " p6 " << p6 << std::endl;
			}
			writeTriangles(ply_outfile, hexCenter, p1, p2, p3, p4, p5, p6);
			globalHexIndex++;
		}
		//std::cerr << std::endl;
	}
	ply_outfile.close();
}
