#include <adt/adt_vector.h>
#include <reflect/reflect_template.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct reflect_template_type
{
	char *name;
	template_type_id type;

	vector parameters;
};

reflect_template template_create(
	const char *name,
	template_type_id type)
{
	reflect_template tpl = malloc(sizeof(struct reflect_template_type));

	if (tpl == NULL)
	{
		return NULL;
	}

	tpl->name = strdup(name);

	if (tpl->name == NULL)
	{
		free(tpl);
		return NULL;
	}

	tpl->type = type;

	tpl->parameters = vector_create(sizeof(char *));

	if (tpl->parameters == NULL)
	{
		free(tpl->name);
		free(tpl);
		return NULL;
	}

	return tpl;
}

static void template_format(
	char *buffer,
	size_t size,
	reflect_template tpl,
	template_argument args[],
	size_t args_size)
{
	if (tpl == NULL)
	{
		return;
	}

	strncat(
		buffer,
		tpl->name,
		size - strlen(buffer) - 1);

	strncat(
		buffer,
		"<",
		size - strlen(buffer) - 1);

	for (size_t i = 0; i < args_size; i++)
	{
		if (i > 0)
		{
			strncat(
				buffer,
				", ",
				size - strlen(buffer) - 1);
		}

		if (args[i].kind == TEMPLATE_ARGUMENT_TYPE)
		{
			const char *name = type_name(args[i].value_type);

			if (name != NULL)
			{
				strncat(
					buffer,
					name,
					size - strlen(buffer) - 1);
			}
		}
		else if (args[i].kind == TEMPLATE_ARGUMENT_TEMPLATE)
		{
			strncat(
				buffer,
				template_name(args[i].value_template),
				size - strlen(buffer) - 1);
		}
	}

	strncat(
		buffer,
		">",
		size - strlen(buffer) - 1);
}

const char *template_name(reflect_template tpl)
{
	return tpl != NULL ? tpl->name : NULL;
}

template_type_id template_type(reflect_template tpl)
{
	return tpl != NULL ? tpl->type : 0;
}

int template_add_parameter(
	reflect_template tpl,
	const char *parameter)
{
	if (tpl == NULL || parameter == NULL)
	{
		return 1;
	}

	char *copy = strdup(parameter);

	if (copy == NULL)
	{
		return 1;
	}

	vector_push_back(
		tpl->parameters,
		&copy);

	return 0;
}

size_t template_parameter_count(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return 0;
	}

	return vector_size(tpl->parameters);
}

const char *template_parameter(
	reflect_template tpl,
	size_t index)
{
	if (tpl == NULL || index >= vector_size(tpl->parameters))
	{
		return NULL;
	}

	char **parameter = (char **)vector_at(tpl->parameters, index);

	return parameter != NULL ? *parameter : NULL;
}

function template_instantiate_function(
	reflect_template tpl,
	template_argument args[],
	size_t size)
{
	if (tpl == NULL ||
		tpl->type != TEMPLATE_TYPE_FUNCTION ||
		args == NULL)
	{
		return NULL;
	}

	if (size != vector_size(tpl->parameters))
	{
		return NULL;
	}

	for (size_t i = 0; i < size; i++)
	{
		const char *parameter =
			template_parameter(tpl, i);

		if (parameter == NULL ||
			args[i].name == NULL ||
			strcmp(parameter, args[i].name) != 0)
		{
			return NULL;
		}

		if (args[i].kind == TEMPLATE_ARGUMENT_TYPE)
		{
			if (args[i].value_type == NULL)
			{
				return NULL;
			}
		}
		else if (args[i].kind == TEMPLATE_ARGUMENT_TEMPLATE)
		{
			if (args[i].value_template == NULL)
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}

	char function_name[256] = { 0 };

	template_format(
		function_name,
		sizeof(function_name),
		tpl,
		args,
		size);

	function f = function_create(
		function_name,
		size,
		NULL,
		NULL);

	if (f == NULL)
	{
		return NULL;
	}

	function_set_template(f, tpl);

	for (size_t i = 0; i < size; i++)
	{
		signature_set(
			function_signature(f),
			i,
			args[i].name,
			args[i].value_type);
	}

	return f;
}

void template_destroy(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return;
	}

	for (size_t i = 0; i < vector_size(tpl->parameters); i++)
	{
		char **parameter =
			(char **)vector_at(tpl->parameters, i);

		if (parameter != NULL)
		{
			free(*parameter);
		}
	}

	vector_destroy(tpl->parameters);

	free(tpl->name);
	free(tpl);
}