/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	reflect_function functions[100];
	int functions_count;
	char *dotnet_root;
	char *dotnet_loader_assembly_path;

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
	corefunction_destroy_execution_result *core_destroy_execution_result;

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
	const CHARSTRING *delegate_destroy_execution_result = W("DestroyExecutionResult");

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
	void destroy_execution_result(execution_result *er);
};

#endif
