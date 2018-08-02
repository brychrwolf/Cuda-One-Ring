#include <iostream>

void func(int** a){
	(*a) = (int*) malloc(5 * sizeof(int));
	for(int i = 0; i < 5; i++)
		(*a)[i] = i + 10;
}

int main(){
	int* a;
	func(&a);
	std::cout << "a = ";
	for(int i = 0; i < 5; i++)
		std::cout << a[i] << " ";
	std::cout << std::endl;
	free(a);
}
