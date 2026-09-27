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

#ifndef _NETCORE_H_
#define _NETCORE_H_

#include <cs_loader/defs.h>
#include <stdlib.h>

class netcore
{
private:
	template <typename T> void **delegate_cast(T *fn)
	{
		union
		{
			void **ptr;
			T *fptr;
		} u;

		u.fptr = fn;

		return u.ptr;
	}

protected:
	reflect_function functions[100]; // TODO: Improve this, make it dynamic
	int functions_count;

	reflect_class classes[100];
	int classes_count;

	reflect_constructor constructors[100];
	int constructors_count;

	reflect_method methods[100];
	int methods_count;

	reflect_attribute attributes[100];
	int attributes_count;

	char *dotnet_root;
	char *dotnet_loader_assembly_path;
	bool initialized;

public:
	execution_path_w *core_execution_path_w;
	execution_path_c *core_execution_path_c;

	load_from_source_w *core_load_from_source_w;
	load_from_source_c *core_load_from_source_c;

	load_from_files_w *core_load_from_files_w;
	load_from_files_c *core_load_from_files_c;

	load_from_assembly_w *core_load_from_assembly_w;
	load_from_assembly_c *core_load_from_assembly_c;

	execute_function_w *execute_w;
	execute_function_c *execute_c;
	execute_function_with_params_w *execute_with_params_w;
	execute_function_with_params_c *execute_with_params_c;
	get_loaded_functions *core_get_functions;
	get_classes_c *core_get_classes;
	get_constructors_c *core_get_constructors;
	get_methods_c *core_get_methods;
	get_attributes_c *core_get_attributes;
	create_object_c *core_create_object;
	invoke_object_c *core_invoke_object;
	invoke_static_c *core_invoke_static;
	get_object_attribute_c *core_get_object_attribute;
	set_object_attribute_c *core_set_object_attribute;
	get_static_attribute_c *core_get_static_attribute;
	set_static_attribute_c *core_set_static_attribute;
	destroy_object_c *core_destroy_object;
	corefunction_destroy_execution_result *core_destroy_execution_result;

	destroy_clr *core_destroy;

	const CHARSTRING *loader_dll = W("CSLoader.dll");
	const CHARSTRING *class_name = W("CSLoader.MetacallEntryPoint");
	const CHARSTRING *assembly_name = W("CSLoader");

	const CHARSTRING *delegate_execution_path_w = W("ExecutionPathW");
	const CHARSTRING *delegate_execution_path_c = W("ExecutionPathW");

	const CHARSTRING *delegate_load_source_w = W("LoadSourceW");
	const CHARSTRING *delegate_load_source_c = W("LoadSourceC");

	const CHARSTRING *delegate_load_files_w = W("LoadFilesW");
	const CHARSTRING *delegate_load_files_c = W("LoadFilesC");

	const CHARSTRING *delegate_load_assembly_w = W("LoadAssemblyW");
	const CHARSTRING *delegate_load_assembly_c = W("LoadAssemblyC");

	const CHARSTRING *delegate_execute_w = W("ExecuteW");
	const CHARSTRING *delegate_execute_c = W("ExecuteC");
	const CHARSTRING *delegate_execute_with_params_w = W("ExecuteWithParamsW");
	const CHARSTRING *delegate_execute_with_params_c = W("ExecuteWithParamsC");
	const CHARSTRING *delegate_get_functions = W("GetFunctions");
	const CHARSTRING *delegate_get_classes = W("GetClasses");
	const CHARSTRING *delegate_get_constructors = W("GetConstructors");
	const CHARSTRING *delegate_get_methods = W("GetMethods");
	const CHARSTRING *delegate_get_attributes = W("GetAttributes");
	const CHARSTRING *delegate_destroy_execution_result = W("DestroyExecutionResult");
	const CHARSTRING *delegate_create_object = W("CreateObjectC");
	const CHARSTRING *delegate_invoke_object = W("InvokeObjectC");
	const CHARSTRING *delegate_invoke_static = W("InvokeStaticC");
	const CHARSTRING *delegate_get_object_attribute =
		W("GetObjectAttributeC");
	const CHARSTRING *delegate_set_object_attribute =
		W("SetObjectAttributeC");
	const CHARSTRING *delegate_get_static_attribute =
		W("GetStaticAttributeC");
	const CHARSTRING *delegate_set_static_attribute =
		W("SetStaticAttributeC");
	const CHARSTRING *delegate_destroy_object = W("DestroyObjectC");

	const CHARSTRING *delegate_destroy = W("Destroy");

	explicit netcore(char *dotnet_root, char *dotnet_loader_assembly_path);
	virtual ~netcore();

	virtual bool start() = 0;
	virtual void stop() = 0;

	bool execution_path(const wchar_t *path);
	bool execution_path(const char *path);

	bool load_source(const wchar_t *source);
	bool load_source(const char *source);

	bool load_files(const wchar_t **source, size_t size);
	bool load_files(const char **source, size_t size);

	bool load_assembly(const wchar_t *source);
	bool load_assembly(const char *source);

	execution_result *execute(const char *function);
	execution_result *execute(const wchar_t *function);

	execution_result *execute_with_params(const char *function, parameters *params);
	execution_result *execute_with_params(const wchar_t *function, parameters *params);

	bool create_delegates();

	virtual bool create_delegate(const CHARSTRING *delegate_name, void **func) = 0;

	reflect_function *get_functions(int *count);

	reflect_class *get_classes(int *count);

	reflect_constructor *get_constructors(int *count);

	reflect_method *get_methods(int *count);

	reflect_attribute *get_attributes(int *count);

	void *create_object(const char *class_name, parameters *params, long size);

	execution_result *invoke_object(void *object, const char *method, parameters *params, long size);

	execution_result *invoke_static(const char *class_name, const char *method, parameters *params, long size);

	execution_result *get_object_attribute(void *object, const char *attribute);

	bool set_object_attribute(void *object, const char *attribute, parameters *param);

	execution_result *get_static_attribute(const char *class_name, const char *attribute);

	bool set_static_attribute(const char *class_name, const char *attribute, parameters *param);

	void destroy_object(void *object);

	void destroy_execution_result(execution_result *er);

	void destroy(void);
};

#endif
