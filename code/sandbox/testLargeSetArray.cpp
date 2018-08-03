#include <iostream>
#include <set>
#include <iterator>
#include <vector>

int main(int argc, char **argv){
	int numVertices = 397210;
	int averageSetSize = 8;
	//std::set<int>* facesOfVertices = (std::set<int>*) malloc(numVertices * (averageSetSize + 1) * sizeof(std::set<int>));	
	std::vector<std::set<int>> facesOfVertices(numVertices);	
	
	facesOfVertices[0].insert(100);
	int found = -1;
	
	//std::set<int>::iterator iter = facesOfVertices[0].find(100);
	//if (iter != facesOfVertices[0].end()){
	//	found = *iter;
	//}
	
	std::cerr << found << std::endl;
}

