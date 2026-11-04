/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_template.h>

#include <reflect/reflect_memory_tracker.h>

#include <threading/threading_atomic_ref_count.h>

#include <adt/adt_vector.h>

#include <format/format_specifier.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct reflect_template_type
{
	char *name;
	enum template_id type;
	vector parameters;
	template_impl impl;
	template_interface interface;
	threading_atomic_ref_count_type ref;
};

reflect_memory_tracker(template_stats);

reflect_template template_create(const char *name, enum template_id type, template_impl impl, template_impl_interface_singleton singleton)
{
	reflect_template tpl;
	size_t name_size;

	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template name, it cannot be NULL");

		return NULL;
	}

	if (type == TEMPLATE_TYPE_INVALID)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) type", name);

		return NULL;
	}

	tpl = malloc(sizeof(struct reflect_template_type));

	if (tpl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) allocation", name);

		return NULL;
	}

	name_size = strlen(name) + 1;

	tpl->name = malloc(sizeof(char) * name_size);

	if (tpl->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template name allocation <%s>", name);

		goto error_name;
	}

	memcpy(tpl->name, name, name_size);

	tpl->type = type;

	tpl->parameters = vector_create(sizeof(char *));

	if (tpl->parameters == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) parameters allocation", name);

		goto error_parameters;
	}

	tpl->impl = impl;
	tpl->interface = singleton ? singleton() : NULL;
	threading_atomic_ref_count_initialize(&tpl->ref);

	if (tpl->interface != NULL && tpl->interface->create != NULL)
	{
		if (tpl->interface->create(tpl, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) create callback <%p>", tpl->name, tpl->interface->create);

			goto error_create;
		}
	}

	reflect_memory_tracker_allocation(template_stats);

	return tpl;

error_create:
	threading_atomic_ref_count_destroy(&tpl->ref);
	vector_destroy(tpl->parameters);
error_parameters:
	free(tpl->name);
error_name:
	free(tpl);
	return NULL;
}

int template_increment_reference(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_increment(&tpl->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_increment(template_stats);

	return 0;
}

int template_decrement_reference(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_decrement(&tpl->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_decrement(template_stats);

	return 0;
}

const char *template_name(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return NULL;
	}

	return tpl->name;
}

enum template_id template_type(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return TEMPLATE_TYPE_INVALID;
	}

	return tpl->type;
}

int template_add_parameter(reflect_template tpl, const char *parameter)
{
	char *copy;
	size_t parameter_size, iterator, size;

	if (tpl == NULL || parameter == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template parameter, the template and the parameter cannot be NULL");

		return 1;
	}

	size = vector_size(tpl->parameters);

	/* Reject duplicated parameter names */
	for (iterator = 0; iterator < size; ++iterator)
	{
		if (strcmp(vector_at_type(tpl->parameters, iterator, char *), parameter) == 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) parameter %s, it is already defined", tpl->name, parameter);

			return 1;
		}
	}

	parameter_size = strlen(parameter) + 1;

	copy = malloc(sizeof(char) * parameter_size);

	if (copy == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) parameter %s allocation", tpl->name, parameter);

		return 1;
	}

	memcpy(copy, parameter, parameter_size);

	vector_push_back(tpl->parameters, &copy);

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

const char *template_parameter(reflect_template tpl, size_t index)
{
	if (tpl == NULL)
	{
		return NULL;
	}

	if (index >= vector_size(tpl->parameters))
	{
		return NULL;
	}

	return vector_at_type(tpl->parameters, index, char *);
}

static int template_arguments_validate(reflect_template tpl, enum template_id type, template_argument args[], size_t size)
{
	size_t iterator;

	if (tpl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template instantiation, the template cannot be NULL");

		return 1;
	}

	if (tpl->type != type)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, the template type %d does not match with %d", tpl->name, (int)tpl->type, (int)type);

		return 1;
	}

	if (args == NULL || size != vector_size(tpl->parameters))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, it expects %" PRIuS " arguments but received %" PRIuS, tpl->name, vector_size(tpl->parameters), args == NULL ? (size_t)0 : size);

		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		const char *parameter = template_parameter(tpl, iterator);

		if (parameter == NULL || args[iterator].name == NULL || strcmp(parameter, args[iterator].name) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %" PRIuS " (%s) does not match with parameter %s", tpl->name, iterator, args[iterator].name != NULL ? args[iterator].name : "NULL", parameter != NULL ? parameter : "NULL");

			return 1;
		}

		if (args[iterator].kind == TEMPLATE_ARGUMENT_TYPE)
		{
			if (args[iterator].data.t == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %s has a NULL type", tpl->name, parameter);

				return 1;
			}
		}
		else if (args[iterator].kind == TEMPLATE_ARGUMENT_TEMPLATE)
		{
			if (args[iterator].data.tpl == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %s has a NULL template", tpl->name, parameter);

				return 1;
			}
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %s has an invalid kind %d", tpl->name, parameter, (int)args[iterator].kind);

			return 1;
		}
	}

	return 0;
}

function template_instantiate_function(reflect_template tpl, template_argument args[], size_t size)
{
	function f;

	if (template_arguments_validate(tpl, TEMPLATE_TYPE_FUNCTION, args, size) != 0)
	{
		return NULL;
	}

	if (tpl->interface == NULL || tpl->interface->instantiate_function == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) function instantiation, the template does not implement instantiate_function", tpl->name);

		return NULL;
	}

	f = tpl->interface->instantiate_function(tpl, tpl->impl, args, size);

	if (f == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) function instantiation, the instantiate_function callback <%p> failed", tpl->name, tpl->interface->instantiate_function);

		return NULL;
	}

	function_set_template(f, tpl);

	return f;
}

klass template_instantiate_class(reflect_template tpl, template_argument args[], size_t size)
{
	klass cls;

	if (template_arguments_validate(tpl, TEMPLATE_TYPE_CLASS, args, size) != 0)
	{
		return NULL;
	}

	if (tpl->interface == NULL || tpl->interface->instantiate_class == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) class instantiation, the template does not implement instantiate_class", tpl->name);

		return NULL;
	}

	cls = tpl->interface->instantiate_class(tpl, tpl->impl, args, size);

	if (cls == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) class instantiation, the instantiate_class callback <%p> failed", tpl->name, tpl->interface->instantiate_class);

		return NULL;
	}

	class_set_template(cls, tpl);

	return cls;
}

void template_destroy(reflect_template tpl)
{
	size_t iterator;

	if (tpl == NULL)
	{
		return;
	}

	if (template_decrement_reference(tpl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in template: %s", tpl->name);
	}

	if (threading_atomic_ref_count_load(&tpl->ref) == 0)
	{
		if (tpl->interface != NULL && tpl->interface->destroy != NULL)
		{
			tpl->interface->destroy(tpl, tpl->impl);
		}

		for (iterator = 0; iterator < vector_size(tpl->parameters); ++iterator)
		{
			char *parameter = vector_at_type(tpl->parameters, iterator, char *);

			if (parameter != NULL)
			{
				free(parameter);
			}
		}

		vector_destroy(tpl->parameters);

		threading_atomic_ref_count_destroy(&tpl->ref);

		free(tpl->name);
		free(tpl);

		reflect_memory_tracker_deallocation(template_stats);
	}
}

void template_stats_debug(void)
{
	reflect_memory_tracker_print(template_stats, "TEMPLATES");
}
