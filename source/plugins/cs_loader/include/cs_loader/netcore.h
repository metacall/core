#pragma once
#ifndef _NETCORE_H_
#define _NETCORE_H_
#include <cs_loader/defs.h>

class netcore
{
protected:

	reflect_function functions[100];
	int functions_count;
public:

	load_function_w * core_load_w;
	load_function_c * core_load_c;
	execute_function_w * execute_w;
	execute_function_c * execute_c;
	execute_function_with_params_w * execute_with_params_w;
	execute_function_with_params_c * execute_with_params_c;
	get_loaded_functions * core_get_functions;

	const CHARSTRING *loader_dll = W("CSLoader.dll");
	const CHARSTRING *class_name = W("CSLoader.Loader");
	const CHARSTRING *assembly_name = W("CSLoader");
	const CHARSTRING *delegate_load_w = W("LoadW");
	const CHARSTRING *delegate_load_c = W("LoadC");
	const CHARSTRING *delegate_execute_w = W("ExecuteW");
	const CHARSTRING *delegate_execute_c = W("ExecuteC");
	const CHARSTRING *delegate_execute_with_params_w = W("ExecuteWithParamsW");
	const CHARSTRING *delegate_execute_with_params_c = W("ExecuteWithParamsC");
	const CHARSTRING *delegate_get_functions = W("GetFunctions");

	explicit netcore();
	virtual ~netcore();

	virtual bool start() = 0;
	virtual void stop() = 0;

	bool load_source(wchar_t * source);
	bool load_source(char * source);

	bool execute(char * function);
	bool execute(wchar_t * function);

	bool execute_with_params(char * function, parameters * params, size_t size);
	bool execute_with_params(wchar_t * function, parameters * params, size_t size);

	bool create_delegates();

	virtual bool create_delegate(const CHARSTRING * delegate_name, void** func) = 0;

	reflect_function * get_functions(int * count);
};

#endif