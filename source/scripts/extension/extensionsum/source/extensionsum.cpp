#include <extensionsum.h>
#include <metacall/metacall.h>

void *sum(size_t argc, void *args[], void *data)
{
	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);
	long result = left + right;

	(void)argc;
	(void)data;

	printf("%ld + %ld = %ld\n", left, right, result);

	return metacall_value_create_long(result);
}

void extensionsum(void *loader, void *context)
{
	enum metacall_value_id arg_types[] = { METACALL_LONG, METACALL_LONG };
	assert(metacall_register_loaderv(loader, context, "sum", sum, METACALL_LONG, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) == 0);
}
