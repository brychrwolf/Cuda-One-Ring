#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

std::vector<std::string> split(const std::string &text, char sep){
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while((end = text.find(sep, start)) != std::string::npos){
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

int main(){
	std::ifstream infile ("test.ply");

	std::string line;
	while(std::getline(infile, line)){
		std::vector<std::string> words = split(line, ' ');
		for(std::string word : words){
			std::cerr << word << " ";
		}
		std::cerr << std::endl;
	}
}
