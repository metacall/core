#include <reflect/reflect_template.h>
#include <adt/adt_vector.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct reflect_template_type
{
	char *name;
	template_type_id type;
    vector parameters;
};

reflect_template reflect_template_create(const char *name, template_type_id type)
{
	reflect_template tpl = malloc(sizeof(struct reflect_template_type));

	if (tpl == NULL)
	{
		return NULL;
	}

	tpl->name = strdup(name);
	tpl->type = type;
    tpl->parameters = vector_create(sizeof(char *));

	return tpl;
}

const char *reflect_template_name(reflect_template tpl)
{
	return tpl != NULL ? tpl->name : NULL;
}

template_type_id reflect_template_type(reflect_template tpl)
{
	return tpl != NULL ? tpl->type : 0;
}

int reflect_template_add_parameter(
	reflect_template tpl,
	const char *parameter
)
{
	if (tpl == NULL || parameter == NULL)
	{
		return 1;
	}

	char *copy = strdup(parameter);

	vector_push_back(
		tpl->parameters,
		&copy
	);

	return 0;
}

size_t reflect_template_parameter_count(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return 0;
	}

	return vector_size(tpl->parameters);
}


const char *reflect_template_parameter(
	reflect_template tpl,
	size_t index
)
{
	if (tpl == NULL || index >= vector_size(tpl->parameters))
	{
		return NULL;
	}

	char **parameter = (char **)vector_at(tpl->parameters, index);

	return parameter != NULL ? *parameter : NULL;
}

function reflect_template_instantiate_function(reflect_template tpl, template_argument args[], size_t size) {

    if (tpl == NULL || tpl->type != TEMPLATE_TYPE_FUNCTION) {
        return NULL;
    }

	if (size != vector_size(tpl->parameters)) {
		return NULL;
	}

	if (args == NULL || size == 0)
{
    return NULL;
}

const char *parameter = reflect_template_parameter(tpl, 0);

if (parameter == NULL || args[0].name == NULL)
{
    return NULL;
}

	if (strcmp(parameter, args[0].name) != 0) {
		return NULL;
	}

	if (args[0].value_type == NULL)
{
    return NULL;
}

char function_name[256];

snprintf(
    function_name,
    sizeof(function_name),
    "%s<%s>",
    tpl->name,
    type_name(args[0].value_type)
);

function f = function_create(function_name, 1, NULL, NULL);

if (f != NULL)
{
    signature_set(
        function_signature(f),
        0,
        "arg",
        args[0].value_type
    );
}

return f;

}

void reflect_template_destroy(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return;
	}

	free(tpl->name);
	free(tpl);
}