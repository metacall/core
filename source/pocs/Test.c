#include <stdio.h>

int adder(int a, int b)
{
	return (a + b);
}

int main()
{
	int sum = adder(2, 3);
	printf("%d", sum);
}