#include <call/argument.h>

typedef struct argument_list_type
{
	argument_type_impl_from_id table_id;
	int count;

} * argument_list;

typedef struct argument_type
{
	enum argument_type_id id;
	argument_type_impl impl;
	void * data;
	size_t size;
} * argument;

argument argument_list_head(argument_list arg_list)
{
	return (argument)(arg_list + 1);
}

argument argument_list_at(argument_list arg_list, int index)
{
	argument arg = argument_list_head(arg_list);

	if (index >= 0 && index < arg_list->count)
	{
		return &arg[index];
	}

	return NULL;
}

size_t argument_primitive_size(enum argument_type_id id)
{
	static size_t primitive_size[TYPE_COUNT] =
	{
		sizeof(char),		// CHAR
		sizeof(unsigned char),	// UCHAR
		sizeof(short),		// SHORT
		sizeof(unsigned short),	// USHORT
		sizeof(int),		// INT
		sizeof(unsigned int),	// UINT
		sizeof(long),		// LONG
		sizeof(unsigned long),	// ULONG
		sizeof(float),		// FLOAT
		sizeof(double),		// DOUBLE
		sizeof(void *)		// PTR
	};

	return primitive_size[id];
}

int argument_list_count(argument_list arg_list)
{
	if (arg_list)
	{
		return arg_list->count;
	}

	return -1;
}

void argument_list_set(argument_list arg_list, int index, enum argument_type_id id, void * data, size_t size)
{
	argument arg = argument_list_at(arg_list, index);

	arg->id = id;
	arg->data = data;
	arg->size = size;
	arg->impl = arg_list->table_id(id);
}

enum argument_type_id argument_list_get_id(argument_list arg_list, int index)
{
	argument arg = argument_list_at(arg_list, index);

	return arg->id;
}

void * argument_list_get_data(argument_list arg_list, int index)
{
	argument arg = argument_list_at(arg_list, index);

	return arg->data;
}

size_t argument_list_get_size(argument_list arg_list, int index)
{
	argument arg = argument_list_at(arg_list, index);

	return arg->size;
}

argument_type_impl argument_list_get_impl(argument_list arg_list, int index)
{
	argument arg = argument_list_at(arg_list, index);

	return arg->impl;
}

void argument_list_destroy(argument_list arg_list)
{
	free(arg_list);
}
