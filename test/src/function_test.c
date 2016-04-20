#include <stdio.h>
#include <call/function.h>

typedef struct example_arg_type
{
	int a;
	float b;
	char c[10];
} * example_arg;

argument_type_impl argument_table_impl_null(enum argument_type_id id)
{
	return NULL;
}

void function_example(char c, int i, void * p)
{
	struct example_arg_type * e = (struct example_arg_type *)p;

	printf("char: %c; int: %d; ptr: %p\n", c, i, p);

	printf("e->a: %d; e->b: %f; e->c: %s\n", e->a, e->b, e->c);
}

int main(int argc, char * argv[])
{
	argument_list signature = argument_list_create_signature(&argument_table_impl_null, 3, TYPE_CHAR, TYPE_INT, TYPE_PTR, sizeof(struct example_arg_type));

	function f = function_create(&function_example, "example", signature);

	{
		char c = 'm';
		int i = 123456789;
		struct example_arg_type e = { 5, 3.3f, "123456789" };

		function_call(f, c, i, &e);
	 }

	function_destroy(f);

	return 0;
}
