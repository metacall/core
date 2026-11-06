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
#include <reflect/reflect_value_type.h>

#include <threading/threading_atomic_ref_count.h>

#include <adt/adt_vector.h>

#include <format/format_specifier.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct template_instance_type
{
	template_argument *args;
	size_t size;
	value v;
};

struct reflect_template_type
{
	char *name;
	type_id kind;
	vector parameters;
	vector instances;
	template_impl impl;
	template_interface interface;
	threading_atomic_ref_count_type ref;
};

static int template_arguments_validate(reflect_template tpl, template_argument args[], size_t size);

static int template_arguments_equal(template_argument left[], template_argument right[], size_t size);

static value template_instance_get(reflect_template tpl, template_argument args[], size_t size);

static template_argument *template_instance_arguments_copy(reflect_template tpl, template_argument args[], size_t size);

static void template_instances_destroy(reflect_template tpl);

reflect_memory_tracker(template_stats);

reflect_template template_create(const char *name, type_id kind, template_impl impl, template_impl_interface_singleton singleton)
{
	reflect_template tpl;
	size_t name_size;

	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template name, it cannot be NULL");

		return NULL;
	}

	if (type_id_invalid(kind) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) type %d", name, (int)kind);

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

	tpl->kind = kind;

	tpl->parameters = vector_create(sizeof(char *));

	if (tpl->parameters == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) parameters allocation", name);

		goto error_parameters;
	}

	tpl->instances = vector_create_type(struct template_instance_type);

	if (tpl->instances == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instances allocation", name);

		goto error_instances;
	}

	tpl->impl = impl;
	tpl->interface = NULL;

	if (singleton != NULL)
	{
		tpl->interface = singleton();
	}

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
	vector_destroy(tpl->instances);
error_instances:
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

type_id template_type(reflect_template tpl)
{
	if (tpl == NULL)
	{
		return TYPE_INVALID;
	}

	return tpl->kind;
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

static int template_arguments_validate(reflect_template tpl, template_argument args[], size_t size)
{
	size_t iterator;

	if (tpl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template instantiation, the template cannot be NULL");

		return 1;
	}

	if (args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, the arguments cannot be NULL", tpl->name);

		return 1;
	}

	if (size != vector_size(tpl->parameters))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, it expects %" PRIuS " arguments but received %" PRIuS, tpl->name, vector_size(tpl->parameters), size);

		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		/* The size matches with the parameters, so the parameter is never NULL */
		const char *parameter = template_parameter(tpl, iterator);

		if (args[iterator].name == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %" PRIuS " has a NULL name, it should be %s", tpl->name, iterator, parameter);

			return 1;
		}

		if (strcmp(parameter, args[iterator].name) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %" PRIuS " (%s) does not match with parameter %s", tpl->name, iterator, args[iterator].name, parameter);

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
		else if (args[iterator].kind == TEMPLATE_ARGUMENT_CLASS)
		{
			if (args[iterator].data.cls == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, argument %s has a NULL class", tpl->name, parameter);

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

static int template_arguments_equal(template_argument left[], template_argument right[], size_t size)
{
	size_t iterator;

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (left[iterator].kind != right[iterator].kind)
		{
			return 1;
		}

		if (left[iterator].kind == TEMPLATE_ARGUMENT_TYPE)
		{
			if (left[iterator].data.t != right[iterator].data.t)
			{
				return 1;
			}
		}
		else if (left[iterator].kind == TEMPLATE_ARGUMENT_TEMPLATE)
		{
			if (left[iterator].data.tpl != right[iterator].data.tpl)
			{
				return 1;
			}
		}
		else if (left[iterator].kind == TEMPLATE_ARGUMENT_CLASS)
		{
			if (left[iterator].data.cls != right[iterator].data.cls)
			{
				return 1;
			}
		}
	}

	return 0;
}

static value template_instance_get(reflect_template tpl, template_argument args[], size_t size)
{
	size_t iterator, instances_size = vector_size(tpl->instances);

	for (iterator = 0; iterator < instances_size; ++iterator)
	{
		struct template_instance_type *instance = vector_at(tpl->instances, iterator);

		if (instance->size == size && template_arguments_equal(instance->args, args, size) == 0)
		{
			return instance->v;
		}
	}

	return NULL;
}

static template_argument *template_instance_arguments_copy(reflect_template tpl, template_argument args[], size_t size)
{
	template_argument *copy = malloc(sizeof(template_argument) * size);
	size_t iterator;

	if (copy == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instance arguments allocation", tpl->name);

		return NULL;
	}

	/* The names point to the parameters of the template, so they live as long as the instance */
	for (iterator = 0; iterator < size; ++iterator)
	{
		copy[iterator] = args[iterator];
		copy[iterator].name = template_parameter(tpl, iterator);
	}

	return copy;
}

value template_instantiate(reflect_template tpl, template_argument args[], size_t size)
{
	struct template_instance_type instance;

	if (template_arguments_validate(tpl, args, size) != 0)
	{
		return NULL;
	}

	instance.v = template_instance_get(tpl, args, size);

	if (instance.v != NULL)
	{
		return instance.v;
	}

	if (tpl->interface == NULL || tpl->interface->instantiate == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, the template does not implement instantiate", tpl->name);

		return NULL;
	}

	instance.args = template_instance_arguments_copy(tpl, args, size);

	if (instance.args == NULL)
	{
		return NULL;
	}

	instance.v = tpl->interface->instantiate(tpl, tpl->impl, args, size);

	if (instance.v == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, the instantiate callback <%p> failed", tpl->name, tpl->interface->instantiate);

		goto error_instantiate;
	}

	if (value_type_id(instance.v) != tpl->kind)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid template (%s) instantiation, the instantiate callback <%p> returned a value of type %s but the template type is %s", tpl->name, tpl->interface->instantiate, type_id_name(value_type_id(instance.v)), type_id_name(tpl->kind));

		goto error_instance_type;
	}

	/* The template owns the value, which holds the reference to the instance until the template is destroyed */
	instance.size = size;

	vector_push_back(tpl->instances, &instance);

	return instance.v;

error_instance_type:
	value_type_destroy(instance.v);
error_instantiate:
	free(instance.args);
	return NULL;
}

static void template_instances_destroy(reflect_template tpl)
{
	size_t iterator, instances_size = vector_size(tpl->instances);

	for (iterator = 0; iterator < instances_size; ++iterator)
	{
		struct template_instance_type *instance = vector_at(tpl->instances, iterator);

		/* Release the reference of the template, the instance survives if someone else holds a reference */
		value_type_destroy(instance->v);

		free(instance->args);
	}

	vector_destroy(tpl->instances);
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
		/* The instances are destroyed before the template implementation, as they may depend on it */
		template_instances_destroy(tpl);

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
