/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

/* -- Headers -- */

#include <environment/environment_variable.h>

#include <stdlib.h>
#include <string.h>

/* -- Methods -- */

char * environment_variable_create(const char * name, const char * default_value)
{
	const char * variable_ptr = getenv(name);

	char * variable;

	size_t length;

	if (variable_ptr == NULL)
	{
		if (default_value == NULL)
		{
			static const char empty_variable[] = "";

			default_value = empty_variable;
		}

		variable_ptr = default_value;
	}

	length = strlen(variable_ptr);

	variable = malloc(sizeof(char) * (length + 1));

	if (variable == NULL)
	{
		return NULL;
	}

	strncpy(variable, variable_ptr, length);

	variable[length] = '\0';

	return variable;
}

const char * environment_variable_get(const char * name, const char * default_value)
{
	const char * variable = getenv(name);

	if (variable != NULL)
	{
		return variable;
	}

	return default_value;
}

void environment_variable_destroy(char * variable)
{
	if (variable != NULL)
	{
		free(variable);
	}
}
