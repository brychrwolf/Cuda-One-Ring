#include <iostream>

int main(){
	int a[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
	int b[3][4] = {
		{10, 11, 12, 13}, 
		{14, 15, 16, 17}, 
		{18, 19, 20, 21}
	};
	int c[3][4] = {};
	
	int j = 0;
	int k = 0;
	for(int i = 0; i < 12; i++){
		std::cout << "a[" << i << "]\t" << a[i] << std::endl;
		j = i / 4;
		k = i % 4;
		std::cout << "b[" << j << "][" << k << "]\t" << b[j][k] << std::endl;
		std::cout << "b[" << i << "]\t" << b[i] << std::endl;
		std::cout << std::endl;
	}
}
