#include <iostream>
#include <string>

int main(){
	std::string test = "test.ply";
	std::cout << test.substr(0, test.length() -4) << std::endl;
}
