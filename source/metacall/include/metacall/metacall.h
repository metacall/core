/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#ifndef METACALL_H
#define METACALL_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include <metacall/metacall_value.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>
#include <stdarg.h>

/* -- Global Variables -- */

extern void * metacall_null_args[1];

/* -- Methods -- */

/**
*  @brief
*    Initialize MetaCall library
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_initialize(void);

/**
*  @brief
*    Amount of function call arguments supported by MetaCall
*
*  @return
*    Number of arguments suported
*/
METACALL_API size_t metacall_args_size(void);

/**
*  @brief
*    Loads a script from file specified by @path
*
*  @param[in] paths
*    Path array of files
*
*  @param[in] size
*    Size of the array @paths
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_file(const char * tag, const char * paths[], size_t size);

/**
*  @brief
*    Loads a script from memory
*
*  @param[in] extension
*    Extension of the script
*
*  @param[in] buffer
*    Memory block representing the string of the script
*
*  @param[in] size
*    Memory block representing the string of the script
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_memory(const char * tag, const char * buffer, size_t size);

/**
*  @brief
*    Loads a package of scrips from file specified by @path into loader defined by @extension
*
*  @param[in] extension
*    Extension of the script
*
*  @param[in] path
*    Path of the package
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_package(const char * tag, const char * path);

/**
*  @brief
*    Call a function anonymously by value array @args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] args
*    Array of pointers to data
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void * metacallv(const char * name, void * args[]);

/**
*  @brief
*    Call a function anonymously by variable arguments @va_args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] va_args
*    Varidic function parameters
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void * metacall(const char * name, ...);

/**
*  @brief
*    Get the function by @name
*
*  @param[in] name
*    Name of the function
*
*  @return
*    Function reference, if null the function does not exist
*/
METACALL_API void * metacall_function(const char * name);

/**
*  @brief
*    Call a function anonymously by value array @args and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] args
*    Array of pointers to data
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void * metacall_function_invokev(void * func, void * args[]);

/**
*  @brief
*    Call a function anonymously by variable arguments @va_args and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void * metacall_function_invoke(void * func, ...);

/**
*  @brief
*    Register a function by name @name and arguments @va_args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] invoke
*    Pointer to function invoke interface
*
*  @param[in] return_type
*    Type of return value
*
*  @param[in] size
*    Number of function arguments
*
*  @param[in] va_args
*    Varidic function parameter types
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API int metacall_register(const char * name, void * (*invoke)(void * []), enum metacall_value_id return_type, size_t size, ...);

/**
*  @brief
*    Destroy MetaCall library
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_destroy(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*/
METACALL_API const char * metacall_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_H */
