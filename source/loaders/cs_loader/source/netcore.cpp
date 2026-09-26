/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
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

#include <cs_loader/netcore.h>

#include <log/log.h>

#include <exception>

netcore::netcore(char *dotnet_root, char *dotnet_loader_assembly_path) :
	dotnet_root(dotnet_root), dotnet_loader_assembly_path(dotnet_loader_assembly_path), initialized(false)
{
}

netcore::~netcore()
{
}

reflect_function *netcore::get_functions(int *count)
{
	this->core_get_functions(&this->functions_count, this->functions);

	*count = this->functions_count;

	return this->functions;
}

reflect_class *netcore::get_classes(int *count)
{
	this->core_get_classes(&this->classes_count, this->classes);

	*count = this->classes_count;

	return this->classes;
}

reflect_constructor *netcore::get_constructors(int *count)
{
	this->core_get_constructors(
		&this->constructors_count,
		this->constructors);

	*count = this->constructors_count;

	return this->constructors;
}

reflect_method *netcore::get_methods(int *count)
{
	this->core_get_methods(&this->methods_count, this->methods);

	*count = this->methods_count;

	return this->methods;
}

reflect_attribute *netcore::get_attributes(int *count)
{
	this->core_get_attributes(
		&this->attributes_count,
		this->attributes);

	*count = this->attributes_count;

	return this->attributes;
}

bool netcore::create_delegates()
{
	if (!this->create_delegate(this->delegate_execution_path_w, delegate_cast(&this->core_execution_path_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execution_path_c, delegate_cast(&this->core_execution_path_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_functions, delegate_cast(&this->core_get_functions)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_classes, delegate_cast(&this->core_get_classes)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_constructors, delegate_cast(&this->core_get_constructors)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_methods, delegate_cast(&this->core_get_methods)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_attributes, delegate_cast(&this->core_get_attributes)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_create_object, delegate_cast(&this->core_create_object)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_invoke_object, delegate_cast(&this->core_invoke_object)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_invoke_static, delegate_cast(&this->core_invoke_static)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_object_attribute, delegate_cast(&this->core_get_object_attribute)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_set_object_attribute, delegate_cast(&this->core_set_object_attribute)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_get_static_attribute, delegate_cast(&this->core_get_static_attribute)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_set_static_attribute, delegate_cast(&this->core_set_static_attribute)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_destroy_object, delegate_cast(&this->core_destroy_object)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_source_w, delegate_cast(&this->core_load_from_source_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_source_c, delegate_cast(&this->core_load_from_source_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_files_w, delegate_cast(&this->core_load_from_files_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_files_c, delegate_cast(&this->core_load_from_files_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_assembly_w, delegate_cast(&this->core_load_from_assembly_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_assembly_c, delegate_cast(&this->core_load_from_assembly_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_w, delegate_cast(&this->execute_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_c, delegate_cast(&this->execute_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_with_params_w, delegate_cast(&this->execute_with_params_w)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_with_params_c, delegate_cast(&this->execute_with_params_c)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_destroy_execution_result, delegate_cast(&this->core_destroy_execution_result)))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_destroy, delegate_cast(&this->core_destroy)))
	{
		return false;
	}

	return true;
}

bool netcore::execution_path(const wchar_t *path)
{
	try
	{
		return this->core_execution_path_w(path) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::execution_path(const char *path)
{
	try
	{
		return this->core_execution_path_c(path) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_source(const wchar_t *source)
{
	try
	{
		return this->core_load_from_source_w(source) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_source(const char *source)
{
	try
	{
		return this->core_load_from_source_c(source) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_files(const wchar_t **source, size_t size)
{
	try
	{
		return this->core_load_from_files_w(source, size) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_files(const char **source, size_t size)
{
	try
	{
		return this->core_load_from_files_c(source, size) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_assembly(const wchar_t *source)
{
	try
	{
		return this->core_load_from_assembly_w(source) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

bool netcore::load_assembly(const char *source)
{
	try
	{
		return this->core_load_from_assembly_c(source) > 0 ? true : false;
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return false;
}

execution_result *netcore::execute(const char *function)
{
	try
	{
		return this->execute_c(function);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return NULL;
}
execution_result *netcore::execute(const wchar_t *function)
{
	try
	{
		return this->execute_w(function);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return NULL;
}
execution_result *netcore::execute_with_params(const char *function, parameters *params)
{
	try
	{
		return this->execute_with_params_c(function, params);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return NULL;
}

execution_result *netcore::execute_with_params(const wchar_t *function, parameters *params)
{
	try
	{
		return this->execute_with_params_w(function, params);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}

	return NULL;
}

void *netcore::create_object(const char *class_name, parameters *params, long size)
{
	try
	{
		return this->core_create_object(class_name, params, size);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return NULL;
}

execution_result *netcore::invoke_object(void *object, const char *method, parameters *params, long size)
{
	try
	{
		return this->core_invoke_object(object, method, params, size);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return NULL;
}

execution_result *netcore::invoke_static(const char *class_name, const char *method, parameters *params, long size)
{
	try
	{
		return this->core_invoke_static(
			class_name,
			method,
			params,
			size);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return NULL;
}

execution_result *netcore::get_object_attribute(void *object, const char *attribute)
{
	try
	{
		return this->core_get_object_attribute(object, attribute);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return NULL;
}

bool netcore::set_object_attribute(void *object, const char *attribute, parameters *param)
{
	try
	{
		return this->core_set_object_attribute(
				   object,
				   attribute,
				   param) == 0;
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return false;
}

execution_result *netcore::get_static_attribute(const char *class_name, const char *attribute)
{
	try
	{
		return this->core_get_static_attribute(
			class_name,
			attribute);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return NULL;
}

bool netcore::set_static_attribute(const char *class_name, const char *attribute, parameters *param)
{
	try
	{
		return this->core_set_static_attribute(
				   class_name,
				   attribute,
				   param) == 0;
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}

	return false;
}

void netcore::destroy_object(void *object)
{
	try
	{
		this->core_destroy_object(object);
	}
	catch (const std::exception &ex)
	{
		log_write(
			"metacall",
			LOG_LEVEL_ERROR,
			"Exception caught: %s",
			ex.what());
	}
}

void netcore::destroy_execution_result(execution_result *er)
{
	try
	{
		this->core_destroy_execution_result(er);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}
}

void netcore::destroy(void)
{
	try
	{
		this->core_destroy();
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Exception caught: %s", ex.what());
	}
}
