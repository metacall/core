#include "extensionsum.h"
#include <metacall/metacall.h>

void *sum(size_t argc, void *args[], void *data)
{
	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);
	long result = left + right;

	(void)argc;
	(void)data;

	printf("%ld + %ld = %lf\n", left, right, result);

	return metacall_value_create_long(result);
}

void *extensionsum(void)
{
	void *f = NULL;
	enum metacall_value_id arg_types[] = { METACALL_LONG, METACALL_LONG };
	metacall_registerv(NULL, sum, &f, METACALL_LONG, sizeof(arg_types) / sizeof(arg_types[0]), arg_types); // TODO: Assert return value == 0
	void *m = metacall_value_create_map(NULL, 1);
	void **m_value = metacall_value_to_map(m);

	m_value[0] = metacall_value_create_array(NULL, 2);

	void **tupla0 = metacall_value_to_array(m_value[0]);

	static const char key0[] = "sum";

	tupla0[0] = metacall_value_create_string(key0, sizeof(key0) - 1);
	tupla0[1] = metacall_value_create_function(f);

	return m;
}
