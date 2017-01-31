/*
 *	MetaCall Quine by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A quine relay proof of concept intercomunicating between multiple programming languages.
 *
 */

/* -- Headers -- */

#include <metacall/metacall.h>

/* -- Methods -- */

int main(int argc, char * argv[])
{
	const char python_script[] =
		"#!/usr/bin/python3.4\n"
		"\n"
		"import _py_portd as metacall\n"
		"\n"
		"def py_quine(script: str) -> int:\n"
		"	print('Python:', script);\n"
		"	return 4;\n";

	void * result;

	(void)argc;
	(void)argv;

	if (metacall_initialize() != 0)
	{
		return 1;
	}

	if (metacall_load_from_memory("py", python_script, sizeof(python_script)) != 0)
	{
		return 1;
	}

	result = metacall("py_quine", "hello");

	if (metacall_value_to_int(result) != 4)
	{
		return 1;
	}

	metacall_value_destroy(result);

	return 0;
}
