/*
 *	MetaCall Quine by Parra Studios
 *	A quine relay proof of concept intercomunicating between multiple programming languages.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

/* -- Headers -- */

#include <metacall/metacall.h>

/* -- Methods -- */

int main(int argc, char * argv[])
{
	const char python_script[] =
		"#!/usr/bin/env python3\n"
		"\n"
		"try:\n"
		"	import _py_port as metacall\n"
		"except ImportError:\n"
		"	import _py_portd as metacall\n"
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

	if (metacall_load_from_memory("py", python_script, sizeof(python_script), NULL) != 0)
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
