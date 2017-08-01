/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading net code at run-time into a process.
*
*/

#ifndef _DEFS_H_
#define _DEFS_H_

#ifndef MAX_LONGPATH
#define MAX_LONGPATH 255
#endif
#include <stdlib.h>
typedef struct {
	short type;
	void * ptr;
} parameters;

typedef struct {
	short type;
	char failed;
	void * ptr;
} execution_result;

typedef struct {
	short type;
	char  name[100];
} reflect_param;

typedef struct {
	short return_type;
	int param_count;
	char  name[100];
	reflect_param pars[10];
} reflect_function;

typedef void(void_func)(void);

typedef char(load_from_source_w)(wchar_t * source);
typedef char(load_from_source_c)(const char * source);

typedef char(load_from_files_w)(wchar_t ** source, size_t size);
typedef char(load_from_files_c)(char ** source, size_t size);

typedef char(load_from_assembly_w)(wchar_t * source);
typedef char(load_from_assembly_c)(const char * source);

typedef void(corefunction_destroy_execution_result)(execution_result* er);
typedef execution_result*(execute_function_c)(char * function);
typedef execution_result*(execute_function_w)(wchar_t * function);
typedef execution_result*(execute_function_with_params_w)(wchar_t * function, parameters *);
typedef execution_result*(execute_function_with_params_c)(char * function, parameters *);
typedef void(get_loaded_functions)(int *, reflect_function*);

#if defined(__linux) | defined( linux)

#define W(str)  str
typedef char CHARSTRING;

#else

#define W(str)  L##str
typedef wchar_t CHARSTRING;
#endif

#endif
