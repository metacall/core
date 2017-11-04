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

#include <metacall/metacall_log.h>
#include <metacall/metacall_allocator.h>
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
*  @param[in] tag
*    Extension of the script
*
*  @param[in] paths
*    Path array of files
*
*  @param[in] size
*    Size of the array @paths
*
*  @param[out] handle
*    Optional pointer to reference of loaded handle
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_file(const char * tag, const char * paths[], size_t size, void ** handle);

/**
*  @brief
*    Loads a script from memory
*
*  @param[in] tag
*    Extension of the script
*
*  @param[in] buffer
*    Memory block representing the string of the script
*
*  @param[in] size
*    Memory block representing the string of the script
*
*  @param[out] handle
*    Optional pointer to reference of loaded handle
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_memory(const char * tag, const char * buffer, size_t size, void ** handle);

/**
*  @brief
*    Loads a package of scrips from file specified by @path into loader defined by @extension
*
*  @param[in] tag
*    Extension of the script
*
*  @param[in] path
*    Path of the package
*
*  @param[out] handle
*    Optional pointer to reference of loaded handle
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_package(const char * tag, const char * path, void ** handle);

/**
*  @brief
*    Loads a a list of scrips from configuration specified by @path into loader
*    with the following format:
*        {
*            "language_id": "<tag>",
*            "path": "<path>",
*            "scripts": [ "<script0>", "<script1>", ..., "<scriptN>" ]
*        }
*
*  @param[in] path
*    Path of the configuration
*
*  @param[out] handle
*    Optional pointer to reference of loaded handle
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_configuration(const char * path, void ** handle);

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
*    Call a function anonymously by type array @ids and variable arguments @va_args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] ids
*    Array of types refered to @va_args
*
*  @param[in] va_args
*    Varidic function parameters
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void * metacallt(const char * name, const enum metacall_value_id ids[], ...);

/**
*  @brief
*    Get the function by @name
*
*  @param[in] name
*    Name of the function
*
*  @return
*    Function reference, null if the function does not exist
*/
METACALL_API void * metacall_function(const char * name);

/**
*  @brief
*    Get the handle by @name
*
*  @param[in] tag
*    Extension of the script
*
*  @param[in] name
*    Name of the handle
*
*  @return
*    Handle reference, null if the function does not exist
*/
METACALL_API void * metacall_handle(const char * tag, const char * name);

/**
*  @brief
*    Get name of a @handle
*
*  @param[in] handle
*    Pointer to the handle to be retrieved
*
*  @return
*    String that references the handle
*/
METACALL_API const char * metacall_handle_id(void * handle);

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
METACALL_API void * metacallfv(void * func, void * args[]);

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
METACALL_API void * metacallf(void * func, ...);

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
METACALL_API int metacall_register(const char * name, void * (*invoke)(void *[]), enum metacall_value_id return_type, size_t size, ...);

/**
*  @brief
*    Provide information about all loaded objects
*
*  @param[out] size
*    Size in bytes of return buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the string
*
*  @return
*    String containing introspection information
*/
METACALL_API char * metacall_inspect(size_t * size, void * allocator);

/**
*  @brief
*    Convert the value @v to serialized string
*
*  @param[in] v
*    Reference to the value
*
*  @param[out] size
*    Size of new allocated string
*
*  @param[in] allocator
*    Pointer to allocator will allocate the string
*
*  @return
*    New allocated string containing stringified value
*/
METACALL_API char * metacall_serialize(void * v, size_t * size, void * allocator);

/**
*  @brief
*    Convert the string @buffer to value
*
*  @param[in] buffer
*    String to be deserialized
*
*  @param[in] size
*    Size of string @buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the value
*
*  @return
*    New allocated value representing the string (must be freed)
*/
METACALL_API void * metacall_deserialize(const char * buffer, size_t size, void * allocator);

/**
*  @brief
*    Clear handle from memory and unload related resources
*
*  @param[in] handle
*    String id reference to the handle to be unloaded
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_clear(void * handle);

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
