#include <iostream>

void ref(int&);
void pointer(int[]);

int main(){
	int a;
	std::cout << "a = " << a << std::endl;
	ref(a);
	std::cout << "a = " << a << std::endl;
	
	int b[2];
	std::cout << "b = {" << b[0] << ", " << b[1] << "}" << std::endl;
	pointer(b);
	std::cout << "b = {" << b[0] << ", " << b[1] << "}" << std::endl;
	
	int numfaces = 25;
	for(int index = 0; index < 100; index++){
		int v = index / numfaces;
		int f = index % numfaces;
		std::cout << "v " << v << " f " << f << std::endl;
	}
}

void ref(int& a){
	a = 5;
	std::cout << "a = " << a << std::endl;
}

void pointer(int b[]){
	b[0] = 1;
	b[1] = 2;
	std::cout << "b = {" << b[0] << ", " << b[1] << "}" << std::endl;
}


