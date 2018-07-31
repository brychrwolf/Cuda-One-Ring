#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

int main(){

	std::ifstream infile ("test.ply");

	std::string line;
	while (std::getline(infile, line))
	{
		std::cerr << line << std::endl;
		//int a, b;
		//if (!(iss >> a >> b)) { break; } // error

		// process pair (a,b)
	}
}
