/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

netcore::netcore(char *dotnet_root, char *dotnet_loader_assembly_path)
{
	this->dotnet_root = dotnet_root;
	this->dotnet_loader_assembly_path = dotnet_loader_assembly_path;
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
