#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <iostream>

const double PI = 4*std::atan(1);
int globalVertex = 1;

void getNumVerticesAndFaces(int numRings, int& numVertices, int& numFaces){
	int nthTriNum = numRings*(numRings+1)/2; //1 3 6 10 15; the nth Triangular Number
	int numHexes = 1 + 6*nthTriNum;
	numFaces = 6*numHexes;
	numVertices = numHexes;
	for(int sideLength = 0; sideLength <= numRings; sideLength++)
		numVertices += 6*(2*sideLength+1);
}

void getPoint(int angle, double center[], double length, double point[]){
	//std::cerr << globalVertex << "\t";
	//std::cerr << angle << " " << center[0] << " " << center[1] << " " << length << std::endl;

	double radian = angle*PI/180;
	point[0] = center[0] + (length * std::cos(radian));
	point[1] = center[1] + (length * std::sin(radian));
	
	for(int i=0;  i<2; i++)
		if(std::abs(point[i]) < 1e-8)
			point[i] = 0;
}

void writePoint(std::ofstream& ply_outfile, double* point){
	//ply_outfile << globalVertex++ << "\t";
	ply_outfile << point[0] << " " << point[1] << " 0 0" << std::endl;
}

int main(int ac, char** av){
	int numRings = 1;
	if(ac > 1) numRings = std::stoi(av[1]);
	int cornerDist = 1;
	double centerDist = std::sqrt(3) * cornerDist;

	int numVertices;
	int numFaces;
	getNumVerticesAndFaces(numRings, numVertices, numFaces);
	

	int ringAngles[6] = {120, 180, 240, 300, 0, 60};
	int cornerAngles[8] = {30, 90, 150, 210, 270, 330, 30, 90}; //covers wrap around for sides 5,6 

	double cursor[2];
	double center[2];
	double corner[2];

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

	//Write middle
	ply_outfile << "0 0 0 1" << std::endl;
	center[0] = 0; center[1] = 0;
	//Write corners of middle hexagon
	for(int i = 0; i < 6; i++){
		getPoint(cornerAngles[i], center, cornerDist, corner);
		writePoint(ply_outfile, corner);
	}
	
	for(int ring = 1; ring <= numRings; ring++){
		//Start on right of center
		cursor[0] = ring*centerDist; cursor[1] = 0;
		for(int sideOfRing = 0; sideOfRing < 6; sideOfRing++){
			for(int hexOnSide = 0; hexOnSide < ring; hexOnSide++){
				//Write the next hex in the direction of this side
				getPoint(ringAngles[sideOfRing], cursor, centerDist, center);
				writePoint(ply_outfile, center);
				//Update cursor
				cursor[0] = center[0]; cursor[1] = center[1];
				//write corners (all hexes get two, corner hexes get 3)
				for(int ci = 0; ci < (hexOnSide<ring-1?2:3); ci++){
					getPoint(cornerAngles[sideOfRing+ci], cursor, cornerDist, corner);
					writePoint(ply_outfile, corner);
				}
			}
		}
	}

	//Triangles
	int ringMax = 0;
	int ringNumFaces;
	int prevRingMax;
	for(int ring = 0; ring <= numRings; ring++){
		std::cerr << "ring " << ring << std::endl;
		std::cerr << "numRings " << numRings << std::endl;
		prevRingMax = ringMax;
		getNumVerticesAndFaces(ring, ringMax, ringNumFaces);
		ringMax -= 1; //offset for 0-index vs 1-index
		int numHexesInRing = ring>0?6*ring:1; //ensure ring 0 -> 1
		std::cerr << "numHexesInRing " << numHexesInRing << std::endl;
		for(int i = 0; i < numHexesInRing; i++){
			int hexCenter = prevRingMax + 1 + i*3;
			if(ring > 0) hexCenter += i%ring==0?0:1;
			//int prevHexMax = 7 + (i-1)*4 + 3;
			int p1 = hexCenter+1;
			int p2 = hexCenter+2;
			int p3 = hexCenter+3;
			int p4 = hexCenter+4;
			int p5 = hexCenter+5;
			int p6 = hexCenter+6;
			ply_outfile << "3 " << hexCenter << " " << p1 << " " << p2 << std::endl;
			ply_outfile << "3 " << hexCenter << " " << p2 << " " << p3 << std::endl;
			ply_outfile << "3 " << hexCenter << " " << p3 << " " << p4 << std::endl;
			ply_outfile << "3 " << hexCenter << " " << p4 << " " << p5 << std::endl;
			ply_outfile << "3 " << hexCenter << " " << p5 << " " << p6 << std::endl;
			ply_outfile << "3 " << hexCenter << " " << p6 << " " << p1 << std::endl;
		}
	} 
	ply_outfile.close();
}
