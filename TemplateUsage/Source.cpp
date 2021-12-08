#include <iostream>
#include "Header.h"

void main()
{
	//调用函数模板
	int n1 = 3, n2 = 6;
	float f1 = 14.3, f2 = 9.5;
	printf("Compare Int:%d\n", compare(n1, n2));
	printf("Compare Float:%d\n", compare(f1, f2));

	//类模板实例化
	Blob<int> squares = { 0, 1, 2, 3, 4, 5 };
	for (size_t i = 0; i != squares.size(); i++)
	{
		squares[i] = i * i;
	}

	//getchar();
}