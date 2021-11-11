#include <stdio.h>

void compiled_print(int a, double b)
{
	printf("a is %d and b is %f\n", a, b);
}

long compiled_sum(long a, long b)
{
	return a + b;
}
