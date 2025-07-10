#include <assert.h>
#include <stdio.h>
#include <string.h>

void compiled_print(int a, double b)
{
	printf("a is %d and b is %f\n", a, b);
}

long compiled_sum(long a, long b)
{
	return a + b;
}

char *return_text(void)
{
	static char input[] = "hello";
	return input;
}

void process_text(char *input)
{
	printf("inside of compiled script '%s'\n", input);
	assert(strcmp(input, "test_test") == 0);
}
