#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef struct data_t
{
	int value;
} * data_ptr_t;

data_ptr_t alloc_data(void)
{
	data_ptr_t ptr = malloc(sizeof(struct data_t));

	ptr->value = 0;

	printf("alloc_data %p\n", ptr);

	return ptr;
}

void alloc_data_args(data_ptr_t *ptr)
{
	*ptr = malloc(sizeof(struct data_t));

	(*ptr)->value = 0;

	printf("alloc_data_args %p\n", *ptr);
	printf("alloc_data_args ref %p\n", ptr);
}

int compare_data_value(data_ptr_t left, data_ptr_t right)
{
	printf("left %p\n", left);
	printf("right %p\n", right);
	assert(left == right);
	return left == right;
}

void set_data_value(data_ptr_t ptr, int value)
{
	printf("set_data_value %p\n", ptr);
	ptr->value = value;
}

int get_data_value(data_ptr_t ptr)
{
	printf("get_data_value %p\n", ptr);
	return ptr->value;
}

void free_data(data_ptr_t ptr)
{
	printf("free_data %p\n", ptr);
	free(ptr);
}

// TODO: When calling from NodeJS it does not work,
// NodeJS emmits double as a call, and this expects long, it needs a casting
/*
void modify_int_ptr(long *l)
{
	printf("l %p\n", l);
	printf("value %d\n", *l);
	assert(*l == 324444L);
	*l = 111L;
}
*/

void modify_int_ptr(double *d)
{
	printf("d %p\n", d);
	printf("value %f\n", *d);
	assert(*d == 324444.0);
	*d = 111.0;
}
