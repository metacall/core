/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading net code at run-time into a process.
*
*/

#ifndef _NETCORE_H_
#define _NETCORE_H_

#include <cs_loader/defs.h>
#include <stdlib.h>

class netcore
{
protected:

	reflect_function functions[100];
	int functions_count;
	char * dotnet_root;
	char * dotnet_loader_assembly_path;

public:
	load_from_source_w * core_load_from_source_w;
	load_from_source_c * core_load_from_source_c;

	load_from_files_w * core_load_from_files_w;
	load_from_files_c * core_load_from_files_c;

	load_from_assembly_w * core_load_from_assembly_w;
	load_from_assembly_c * core_load_from_assembly_c;

	execute_function_w * execute_w;
	execute_function_c * execute_c;
	execute_function_with_params_w * execute_with_params_w;
	execute_function_with_params_c * execute_with_params_c;
	get_loaded_functions * core_get_functions;
	corefunction_destroy_execution_result * core_destroy_execution_result;

	const CHARSTRING *loader_dll = W("CSLoader.dll");
	const CHARSTRING *class_name = W("CSLoader.Loader");
	const CHARSTRING *assembly_name = W("CSLoader");

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

	explicit netcore(char * dotnet_root, char * dotnet_loader_assembly_path);
	virtual ~netcore();

	virtual bool start() = 0;
	virtual void stop() = 0;

	bool load_source(wchar_t * source);
	bool load_source(char * source);

	bool load_files(wchar_t ** source, size_t size);
	bool load_files(char ** source, size_t size);

	bool load_assembly(wchar_t * source);
	bool load_assembly(char * source);

	execution_result* execute(char * function);
	execution_result* execute(wchar_t * function);

	execution_result* execute_with_params(char * function, parameters * params);
	execution_result* execute_with_params(wchar_t * function, parameters * params);

	bool create_delegates();

	virtual bool create_delegate(const CHARSTRING * delegate_name, void** func) = 0;

	reflect_function * get_functions(int * count);
	void destroy_execution_result(execution_result* er);
};

#endif
