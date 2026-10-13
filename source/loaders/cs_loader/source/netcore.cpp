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

#include <limits>

namespace
{
template <typename Record, typename Writer>
Record *cs_get_metadata(Writer writer, std::vector<Record> &records, int &stored_count, int *count, const char *label)
{
	if (count == nullptr)
	{
		return nullptr;
	}

	/* Failure remains distinguishable from successful empty metadata. */
	*count = -1;

	if (writer == nullptr)
	{
		return nullptr;
	}

	try
	{
		const int maximum_attempts = 3;

		for (int attempt = 0; attempt < maximum_attempts; ++attempt)
		{
			int required = -1;
			int status = writer(&required, nullptr, 0);

			if (status != METADATA_SUCCESS || required < 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR,
					"Failed to query C# %s metadata", label);
				return nullptr;
			}

			std::vector<Record> next_records;
			const size_t required_size = static_cast<size_t>(required);

			if (required_size > next_records.max_size() ||
				required_size > std::numeric_limits<size_t>::max() / sizeof(Record))
			{
				log_write("metacall", LOG_LEVEL_ERROR,
					"C# %s metadata size exceeds native limits", label);
				return nullptr;
			}

			if (required == 0)
			{
				records.swap(next_records);
				stored_count = 0;
				*count = 0;
				return nullptr;
			}

			/* The managed writer needs actual elements, not reserved capacity. */
			next_records.resize(required_size);

			int written = -1;

			status = writer(
				&written,
				next_records.data(),
				required);

			if (status == METADATA_INSUFFICIENT_CAPACITY)
			{
				if (written <= required)
				{
					log_write("metacall", LOG_LEVEL_ERROR,
						"Invalid C# %s metadata capacity response", label);
					return nullptr;
				}

				/* Discard this buffer and query again on the next attempt. */
				continue;
			}

			if (status != METADATA_SUCCESS ||
				written < 0 ||
				written > required)
			{
				log_write("metacall", LOG_LEVEL_ERROR,
					"Failed to fill C# %s metadata", label);
				return nullptr;
			}

			next_records.resize(static_cast<size_t>(written));

			/* Publish only a completely successful result. */
			records.swap(next_records);
			stored_count = written;
			*count = written;

			return records.empty() ? nullptr : records.data();
		}

		log_write("metacall", LOG_LEVEL_ERROR,
			"C# %s metadata capacity retry limit reached", label);
	}
	catch (const std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR,
			"Failed to allocate or retrieve C# %s metadata: %s",
			label, ex.what());
	}
	catch (...)
	{
		log_write("metacall", LOG_LEVEL_ERROR,
			"Unexpected failure retrieving C# %s metadata", label);
	}

	return nullptr;
}
} // namespace

netcore::netcore(char *dotnet_root, char *dotnet_loader_assembly_path) :
	dotnet_root(dotnet_root), dotnet_loader_assembly_path(dotnet_loader_assembly_path), initialized(false)
{
}

netcore::~netcore()
{
}

reflect_function *netcore::get_functions(int *count)
{
	return cs_get_metadata(this->core_get_functions, this->functions, this->functions_count, count, "function");
}

reflect_class *netcore::get_classes(int *count)
{
	return cs_get_metadata(this->core_get_classes, this->classes, this->classes_count, count, "class");
}

reflect_constructor *netcore::get_constructors(int *count)
{
	return cs_get_metadata(
		this->core_get_constructors, this->constructors, this->constructors_count, count, "constructor");
}

reflect_method *netcore::get_methods(int *count)
{
	return cs_get_metadata(this->core_get_methods, this->methods, this->methods_count, count, "method");
}

reflect_attribute *netcore::get_attributes(int *count)
{
	return cs_get_metadata(this->core_get_attributes, this->attributes, this->attributes_count, count, "attribute");
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
