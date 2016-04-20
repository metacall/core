#include <stdio.h>
#include <call/argument.h>

struct argument_example
{
	float a;
	int b;
	char c[10];
};

argument_type_impl argument_table_null(enum argument_type_id id)
{
	return NULL;
}

int main(int argc, char * argv[])
{
	argument_list arg_list = argument_list_create(&argument_table_null, 5);

	if (arg_list)
	{
		int i;

		int first;
		float second;
		char third[36];
		int fourth[3];
		struct argument_example example[2];

		printf("Argument list created successfully\n");

		argument_list_set(arg_list, 0, TYPE_INT, &first, argument_primitive_size(TYPE_INT));
		argument_list_set(arg_list, 1, TYPE_FLOAT, &second, argument_primitive_size(TYPE_INT));
		argument_list_set(arg_list, 2, TYPE_PTR, &third, argument_primitive_size(TYPE_CHAR) * 36);
		argument_list_set(arg_list, 3, TYPE_PTR, &fourth, argument_primitive_size(TYPE_INT) * 3);
		argument_list_set(arg_list, 4, TYPE_PTR, &example, sizeof(struct argument_example) * 2);

		for (i = 0; i < argument_list_count(arg_list); ++i)
		{
			printf("argument[%d]: id %d, data %p, size %zu, impl %p\n",
				i,
				argument_list_get_id(arg_list, i),
				argument_list_get_data(arg_list, i),
				argument_list_get_size(arg_list, i),
				argument_list_get_impl(arg_list, i));
		}

		argument_list_destroy(arg_list);

		return 0;
	}

	return 1;
}
