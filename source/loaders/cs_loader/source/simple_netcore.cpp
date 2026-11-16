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

#include <cs_loader/simple_netcore.h>

#include <cs_loader/netcore.h>
#if defined(__linux) | defined(linux)
	#include <cs_loader/netcore_linux.h>
#else
	#include <cs_loader/netcore_win.h>
#endif

netcore_handle simple_netcore_create(char *dotnet_root, char *dotnet_loader_assembly_path)
{
#if defined(__linux) | defined(linux)
	netcore *core = new netcore_linux(dotnet_root, dotnet_loader_assembly_path);
#else
	netcore *core = new netcore_win(dotnet_root, dotnet_loader_assembly_path);
#endif

	bool result = core->start();

	if (result == false)
	{
		delete core;

		return (netcore_handle)NULL;
	}

	return (netcore_handle)core;
}

int simple_netcore_get_functions(
	netcore_handle handle, reflect_function **functions, int *count)
{
	if (functions == nullptr || count == nullptr)
	{
		return 1;
	}

	*functions = nullptr;
	*count = 0;

	if (handle == nullptr)
	{
		return 1;
	}

	netcore *core = (netcore *)handle;
	int retrieved_count = -1;
	reflect_function *retrieved = core->get_functions(&retrieved_count);

	if (retrieved_count < 0 ||
		(retrieved_count > 0 && retrieved == nullptr))
	{
		return 1;
	}

	*functions = retrieved;
	*count = retrieved_count;
	return 0;
}

int simple_netcore_get_classes(netcore_handle handle, reflect_class **classes, int *count)
{
	if (classes == nullptr || count == nullptr)
	{
		return 1;
	}

	*classes = nullptr;
	*count = 0;

	if (handle == nullptr)
	{
		return 1;
	}

	netcore *core = (netcore *)handle;
	int retrieved_count = -1;
	reflect_class *retrieved = core->get_classes(&retrieved_count);

	if (retrieved_count < 0 || (retrieved_count > 0 && retrieved == nullptr))
	{
		return 1;
	}

	*classes = retrieved;
	*count = retrieved_count;

	return 0;
}

int simple_netcore_get_constructors(netcore_handle handle, reflect_constructor **constructors, int *count)
{
	if (constructors == nullptr || count == nullptr)
	{
		return 1;
	}

	*constructors = nullptr;
	*count = 0;

	if (handle == nullptr)
	{
		return 1;
	}

	netcore *core = (netcore *)handle;
	int retrieved_count = -1;
	reflect_constructor *retrieved = core->get_constructors(&retrieved_count);

	if (retrieved_count < 0 || (retrieved_count > 0 && retrieved == nullptr))
	{
		return 1;
	}

	*constructors = retrieved;
	*count = retrieved_count;

	return 0;
}

int simple_netcore_get_methods(netcore_handle handle, reflect_method **methods, int *count)
{
	if (methods == nullptr || count == nullptr)
	{
		return 1;
	}

	*methods = nullptr;
	*count = 0;

	if (handle == nullptr)
	{
		return 1;
	}

	netcore *core = (netcore *)handle;
	int retrieved_count = -1;
	reflect_method *retrieved = core->get_methods(&retrieved_count);

	if (retrieved_count < 0 || (retrieved_count > 0 && retrieved == nullptr))
	{
		return 1;
	}

	*methods = retrieved;
	*count = retrieved_count;

	return 0;
}

int simple_netcore_get_attributes(netcore_handle handle, reflect_attribute **attributes, int *count)
{
	if (attributes == nullptr || count == nullptr)
	{
		return 1;
	}

	*attributes = nullptr;
	*count = 0;

	if (handle == nullptr)
	{
		return 1;
	}

	netcore *core = (netcore *)handle;
	int retrieved_count = -1;
	reflect_attribute *retrieved = core->get_attributes(&retrieved_count);

	if (retrieved_count < 0 || (retrieved_count > 0 && retrieved == nullptr))
	{
		return 1;
	}

	*attributes = retrieved;
	*count = retrieved_count;

	return 0;
}

void *simple_netcore_create_object(netcore_handle handle, const char *class_name, parameters *params, long size)
{
	netcore *core = (netcore *)handle;

	return core->create_object(class_name, params, size);
}

execution_result *simple_netcore_invoke_object(netcore_handle handle, void *object, const char *method, parameters *params, long size)
{
	netcore *core = (netcore *)handle;

	return core->invoke_object(object, method, params, size);
}

execution_result *simple_netcore_invoke_static(netcore_handle handle, const char *class_name, const char *method, parameters *params, long size)
{
	netcore *core = (netcore *)handle;

	return core->invoke_static(
		class_name,
		method,
		params,
		size);
}

execution_result *simple_netcore_get_object_attribute(netcore_handle handle, void *object, const char *attribute)
{
	netcore *core = (netcore *)handle;

	return core->get_object_attribute(object, attribute);
}

int simple_netcore_set_object_attribute(netcore_handle handle, void *object, const char *attribute, parameters *param)
{
	netcore *core = (netcore *)handle;

	return core->set_object_attribute(
			   object,
			   attribute,
			   param) ?
			   0 :
			   1;
}

execution_result *simple_netcore_get_static_attribute(netcore_handle handle, const char *class_name, const char *attribute)
{
	netcore *core = (netcore *)handle;

	return core->get_static_attribute(
		class_name,
		attribute);
}

int simple_netcore_set_static_attribute(netcore_handle handle, const char *class_name, const char *attribute, parameters *param)
{
	netcore *core = (netcore *)handle;

	return core->set_static_attribute(
			   class_name,
			   attribute,
			   param) ?
			   0 :
			   1;
}

void simple_netcore_destroy_object(netcore_handle handle, void *object)
{
	netcore *core = (netcore *)handle;

	core->destroy_object(object);
}

int simple_netcore_load_script_from_files(netcore_handle handle, const char *files[MAX_FILES], size_t size)
{
	netcore *core = (netcore *)handle;

	return core->load_files(files, size) == true ? 0 : 1;
}

int simple_netcore_execution_path(netcore_handle handle, const char *path)
{
	netcore *core = (netcore *)handle;

	return core->execution_path(path) == true ? 0 : 1;
}

int simple_netcore_load_script_from_assembly(netcore_handle handle, const char *file)
{
	netcore *core = (netcore *)handle;

	return core->load_assembly(file) == true ? 0 : 1;
}

int simple_netcore_load_script_from_memory(netcore_handle handle, const char *buffer, size_t size)
{
	netcore *core = (netcore *)handle;

	(void)size;

	return core->load_source((char *)buffer) == true ? 0 : 1;
}

execution_result *simple_netcore_invoke(netcore_handle handle, const char *func)
{
	netcore *core = (netcore *)handle;

	return core->execute((char *)func);
}

execution_result *simple_netcore_invoke_with_params(netcore_handle handle, const char *func, parameters *params)
{
	netcore *core = (netcore *)handle;

	return core->execute_with_params((char *)func, params);
}

void simple_netcore_destroy_execution_result(netcore_handle handle, execution_result *er)
{
	netcore *core = (netcore *)handle;

	core->destroy_execution_result(er);
}

void simple_netcore_destroy(netcore_handle handle)
{
	netcore *core = (netcore *)handle;

	core->destroy();

	delete core;
}
