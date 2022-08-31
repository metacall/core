/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_H
#define METACALL_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include <metacall/metacall_allocator.h>
#include <metacall/metacall_def.h>
#include <metacall/metacall_error.h>
#include <metacall/metacall_log.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall_version.h>

#ifdef METACALL_FORK_SAFE
	#include <metacall/metacall_fork.h>
#endif /* METACALL_FORK_SAFE */

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

/* -- Definitions -- */

#define METACALL_FLAGS_FORK_SAFE 0x01 << 0x00

/* -- Forward Declarations -- */

struct metacall_initialize_configuration_type;

/* -- Type Definitions -- */

struct metacall_initialize_configuration_type
{
	char *tag;
	void *options; // TODO: We should use a MetaCall value MAP here and merge it with the configuration.
				   // By this way loaders will be able to access this information in the backend and we
				   // can use a weak API in order to implement this successfully
};

typedef void *(*metacall_await_callback)(void *, void *);

typedef struct metacall_await_callbacks_type
{
	metacall_await_callback resolve;
	metacall_await_callback reject;

} metacall_await_callbacks;

struct metacall_version_type
{
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
	const char *revision;
	const char *str;
	const char *name;
};

/* -- Global Variables -- */

METACALL_API extern void *metacall_null_args[1];

/* -- Methods -- */

/**
*  @brief
*    Returns default serializer used by MetaCall
*
*  @return
*    Name of the serializer to be used with serialization methods
*/
METACALL_API const char *metacall_serial(void);

/**
*  @brief
*    Disables MetaCall logs, must be called before @metacall_initialize.
*
*   When initializing MetaCall, it initializes a default logs to stdout
*   if none was defined. If you want to benchmark or simply disable this
*   default logs, you can call to this function before @metacall_initialize.
*/
METACALL_API void metacall_log_null(void);

/**
*  @brief
*    Flags to be set in MetaCall library
*
*  @param[in] flags
*    Combination of flags referring to definitions METACALL_FLAGS_*
*/
METACALL_API void metacall_flags(int flags);

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
*    Initialize MetaCall library with configuration arguments
*
*  @param[in] initialize_config
*    Extension of the script to be loaded in memory with data to be injected
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_initialize_ex(struct metacall_initialize_configuration_type initialize_config[]);

/**
*  @brief
*    Initialize MetaCall application arguments
*
*  @param[in] argc
*    Number of additional parameters to be passed to the runtime when initializing
*
*  @param[in] argv
*    Additional parameters to be passed to the runtime when initializing (when using MetaCall as an application)
*/
METACALL_API void metacall_initialize_args(int argc, char *argv[]);

/**
*  @brief
*    Get the number of arguments in which MetaCall was initialized
*
*  @return
*    An integer equal or greater than zero
*/
METACALL_API int metacall_argc(void);

/**
*  @brief
*    Get the arguments in which MetaCall was initialized
*
*  @return
*    A pointer to an array of strings with the additional arguments
*/
METACALL_API char **metacall_argv(void);

/**
*  @brief
*    Check if script context is loaded by @tag
*
*  @param[in] tag
*    Extension of the script
*
*  @return
*    Zero if context is initialized, different from zero otherwise
*/
METACALL_API int metacall_is_initialized(const char *tag);

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
*    Set a execution path defined by @path to the extension script @tag
*
*  @param[in] tag
*    Extension of the script
*
*  @param[in] path
*    Path to be loaded
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_execution_path(const char *tag, const char *path);

/**
*  @brief
*    Set a execution path defined by @path to the extension script @tag with length
*
*  @param[in] tag
*    Extension of the script
*
*  @param[in] tag_length
*    Length of the extension of the tag
*
*  @param[in] path
*    Path to be loaded
*
*  @param[in] path_length
*    Length of the path
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_execution_path_s(const char *tag, size_t tag_length, const char *path, size_t path_length);

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
*  @param[inout] handle
*    Optional pointer to reference of loaded handle. If the parameter is NULL, the symbols loaded are
*    propagated to the loader scope (i.e they will share the scope between all previously loaded files and they can collide).
*    Otherwise, if we pass a void* pointer set to NULL, it will behave as output parameter, obtaining the reference to the
*    created handle, which can be later on used for calling to functions of that handle. The symbols will not be propagated
*    to the loader scope and they will be private (this prevents collisions). The last case is if we pass an already allocated
*    handle (i.e a void* pointer pointing to an previously loaded handle), then in this case, the symbols loaded will be propagated
*    to the previously allocated handle, and it will behave as a in parameter.
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_file(const char *tag, const char *paths[], size_t size, void **handle);

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
*  @param[inout] handle
*    Optional pointer to reference of loaded handle. If the parameter is NULL, the symbols loaded are
*    propagated to the loader scope (i.e they will share the scope between all previously loaded files and they can collide).
*    Otherwise, if we pass a void* pointer set to NULL, it will behave as output parameter, obtaining the reference to the
*    created handle, which can be later on used for calling to functions of that handle. The symbols will not be propagated
*    to the loader scope and they will be private (this prevents collisions). The last case is if we pass an already allocated
*    handle (i.e a void* pointer pointing to an previously loaded handle), then in this case, the symbols loaded will be propagated
*    to the previously allocated handle, and it will behave as a in parameter.
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_memory(const char *tag, const char *buffer, size_t size, void **handle);

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
*  @param[inout] handle
*    Optional pointer to reference of loaded handle. If the parameter is NULL, the symbols loaded are
*    propagated to the loader scope (i.e they will share the scope between all previously loaded files and they can collide).
*    Otherwise, if we pass a void* pointer set to NULL, it will behave as output parameter, obtaining the reference to the
*    created handle, which can be later on used for calling to functions of that handle. The symbols will not be propagated
*    to the loader scope and they will be private (this prevents collisions). The last case is if we pass an already allocated
*    handle (i.e a void* pointer pointing to an previously loaded handle), then in this case, the symbols loaded will be propagated
*    to the previously allocated handle, and it will behave as a in parameter.
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_package(const char *tag, const char *path, void **handle);

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
*  @param[inout] handle
*    Optional pointer to reference of loaded handle. If the parameter is NULL, the symbols loaded are
*    propagated to the loader scope (i.e they will share the scope between all previously loaded files and they can collide).
*    Otherwise, if we pass a void* pointer set to NULL, it will behave as output parameter, obtaining the reference to the
*    created handle, which can be later on used for calling to functions of that handle. The symbols will not be propagated
*    to the loader scope and they will be private (this prevents collisions). The last case is if we pass an already allocated
*    handle (i.e a void* pointer pointing to an previously loaded handle), then in this case, the symbols loaded will be propagated
*    to the previously allocated handle, and it will behave as a in parameter.
*
*  @param[in] allocator
*    Pointer to allocator will allocate the configuration
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_load_from_configuration(const char *path, void **handle, void *allocator);

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
METACALL_API void *metacallv(const char *name, void *args[]);

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
*  @param[in] size
*    Number of elements of the call
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallv_s(const char *name, void *args[], size_t size);

/**
*  @brief
*    Call a function anonymously by handle @handle value array @args
*    This function allows to avoid name collisions when calling functions by name
*
*  @param[in] handle
*    Handle where the function belongs
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
METACALL_API void *metacallhv(void *handle, const char *name, void *args[]);

/**
*  @brief
*    Call a function anonymously by handle @handle value array @args
*    This function allows to avoid name collisions when calling functions by name
*    Includes @size in order to allow variadic arguments or safe calls
*
*  @param[in] handle
*    Handle where the function belongs
*
*  @param[in] name
*    Name of the function
*
*  @param[in] args
*    Array of pointers to data
*
*  @param[in] size
*    Number of elements of the call
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallhv_s(void *handle, const char *name, void *args[], size_t size);

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
METACALL_API void *metacall(const char *name, ...);

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
METACALL_API void *metacallt(const char *name, const enum metacall_value_id ids[], ...);

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
*  @param[in] size
*    Number of elements of the call
*
*  @param[in] va_args
*    Varidic function parameters
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallt_s(const char *name, const enum metacall_value_id ids[], size_t size, ...);

/**
*  @brief
*    Call a function anonymously by type array @ids and variable arguments @va_args
*
*  @param[in] handle
*    Pointer to the handle returned by metacall_load_from_{file, memory, package}
*
*  @param[in] name
*    Name of the function
*
*  @param[in] ids
*    Array of types refered to @va_args
*
*  @param[in] size
*    Number of elements of the call
*
*  @param[in] va_args
*    Varidic function parameters
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallht_s(void *handle, const char *name, const enum metacall_value_id ids[], size_t size, ...);

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
METACALL_API void *metacall_function(const char *name);

/**
*  @brief
*    Get the function by @name from @handle
*
*  @param[in] handle
*    Pointer to the handle returned by metacall_load_from_{file, memory, package}
*
*  @param[in] name
*    Name of the function
*
*  @return
*    Function reference, null if the function does not exist
*/
METACALL_API void *metacall_handle_function(void *handle, const char *name);

/**
*  @brief
*    Get the function parameter type id
*
*  @param[in] func
*    The pointer to the function obtained from metacall_function
*
*  @param[in] parameter
*    The index of the parameter to be retrieved
*
*  @param[out] id
*    The parameter type id that will be returned
*
*  @return
*    Return 0 if the @parameter index exists and @func is valid, 1 otherwhise
*/
METACALL_API int metacall_function_parameter_type(void *func, size_t parameter, enum metacall_value_id *id);

/**
*  @brief
*    Get the function return type id
*
*  @param[in] func
*    The pointer to the function obtained from metacall_function
*
*
*  @param[out] id
*    The value id of the return type of the function @func
*
*  @return
*    Return 0 if the @func is valid, 1 otherwhise
*/
METACALL_API int metacall_function_return_type(void *func, enum metacall_value_id *id);

/**
*  @brief
*    Get minimun mumber of arguments accepted by function @func
*
*  @param[in] func
*    Function reference
*
*  @return
*    Return mumber of arguments
*/
METACALL_API size_t metacall_function_size(void *func);

/**
*  @brief
*    Check if the function @func is asynchronous or synchronous
*
*  @param[in] func
*    Function reference
*
*  @return
*    Return 0 if it is syncrhonous, 1 if it is asynchronous and -1 if the function is NULL
*/
METACALL_API int metacall_function_async(void *func);

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
METACALL_API void *metacall_handle(const char *tag, const char *name);

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
METACALL_API const char *metacall_handle_id(void *handle);

/**
*  @brief
*    Return a value representing the handle as a map of functions (or values)
*
*  @param[in] handle
*    Reference to the handle to be described
*
*  @return
*    A value of type map on success, null otherwise
*/
METACALL_API void *metacall_handle_export(void *handle);

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
METACALL_API void *metacallfv(void *func, void *args[]);

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
*  @param[in] size
*    Number of function arguments
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallfv_s(void *func, void *args[], size_t size);

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
METACALL_API void *metacallf(void *func, ...);

/**
*  @brief
*    Call a function anonymously by function @func and serial @buffer of size @size
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] buffer
*    String representing an array to be deserialized into arguments of the function
*
*  @param[in] size
*    Size of string @buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the value
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallfs(void *func, const char *buffer, size_t size, void *allocator);

/**
*  @brief
*    Call a function anonymously by value map (@keys -> @values) and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] keys
*    Array of values representing argument keys
*
*  @param[in] values
*    Array of values representing argument values data
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallfmv(void *func, void *keys[], void *values[]);

/**
*  @brief
*    Call a function anonymously by function @func and serial @buffer of size @size
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] buffer
*    String representing a map to be deserialized into arguments of the function
*
*  @param[in] size
*    Size of string @buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the value
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallfms(void *func, const char *buffer, size_t size, void *allocator);

/**
*  @brief
*    Register a function by name @name and arguments @va_args
*
*  @param[in] name
*    Name of the function (if it is NULL, function is not registered into host scope)
*
*  @param[in] invoke
*    Pointer to function invoke interface (argc, argv, data)
*
*  @param[out] func
*    Will set the pointer to the function if the parameter is not null
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
METACALL_API int metacall_register(const char *name, void *(*invoke)(size_t, void *[], void *), void **func, enum metacall_value_id return_type, size_t size, ...);

/**
*  @brief
*    Register a function by name @name and arguments @types
*
*  @param[in] name
*    Name of the function (if it is NULL, function is not registered into host scope)
*
*  @param[in] invoke
*    Pointer to function invoke interface (argc, argv, data)
*
*  @param[out] func
*    Will set the pointer to the function if the parameter is not null
*
*  @param[in] return_type
*    Type of return value
*
*  @param[in] size
*    Number of function arguments
*
*  @param[in] types
*    List of parameter types
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API int metacall_registerv(const char *name, void *(*invoke)(size_t, void *[], void *), void **func, enum metacall_value_id return_type, size_t size, enum metacall_value_id types[]);

/**
*  @brief
*    Register a function by name @name and arguments @types
*
*  @param[in] loader
*    Opaque pointer to the loader in which you want to register the function (this allows to register the function into a different loader than the host)
*
*  @param[in] context
*    Opaque pointer to the context in which you want to register the function (if it is NULL, it will be defined on the global scope of the loader)
*
*  @param[in] name
*    Name of the function (if it is NULL, function is not registered into host scope)
*
*  @param[in] invoke
*    Pointer to function invoke interface (argc, argv, data)
*
*  @param[in] return_type
*    Type of return value
*
*  @param[in] size
*    Number of function arguments
*
*  @param[in] types
*    List of parameter types
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API int metacall_register_loaderv(void *loader, void *context, const char *name, void *(*invoke)(size_t, void *[], void *), enum metacall_value_id return_type, size_t size, enum metacall_value_id types[]);

/**
*  @brief
*    Executes an asynchronous call to the function and registers a callback to be executed when a future is resolved (it does block)
*
*  @param[in] name
*    The name of the function to be called asynchronously
*
*  @param[in] args
*    Array of pointers to the values to be passed to the function
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacall_await(const char *name, void *args[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Awaits for a promise and registers a callback to be executed when a future is resolved
*
*  @param[in] f
*    The pointer to the future
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacall_await_future(void *f, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Executes an asynchronous call to the function and registers a callback to be executed when a future is resolved (it does block)
*
*  @param[in] name
*    The name of the function to be called asynchronously
*
*  @param[in] args
*    Array of pointers to the values to be passed to the function
*
*  @param[in] size
*    Number of elements of the array @args
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacall_await_s(const char *name, void *args[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by value array @args and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] args
*    Array of pointers to values
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfv_await(void *func, void *args[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by value array @args and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] args
*    Array of pointers to values
*
*  @param[in] size
*    Number of elements of the array @args
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfv_await_s(void *func, void *args[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by value array @args and function @func (offered without function pointers for languages without support to function pointers)
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] args
*    Array of pointers to values
*
*  @param[in] size
*    Number of elements of the array @args
*
*  @param[in] cb
*    Pointer to struct containing the function pointers to reject and resolve that will be executed when task completion or error
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfv_await_struct_s(void *func, void *args[], size_t size, metacall_await_callbacks cb, void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by value map (@keys -> @values) and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] keys
*    Array of values representing argument keys
*
*  @param[in] values
*    Array of values representing argument values data
*
*  @param[in] size
*    Number of elements of the arrays @keys and @values
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfmv_await(void *func, void *keys[], void *values[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by value map (@keys -> @values) and function @func
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] keys
*    Array of values representing argument keys
*
*  @param[in] values
*    Array of values representing argument values data
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfmv_await_s(void *func, void *keys[], void *values[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by function @func and serial @buffer of size @size
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] buffer
*    String representing an array to be deserialized into arguments of the function
*
*  @param[in] size
*    Size of string @buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the value
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfs_await(void *func, const char *buffer, size_t size, void *allocator, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Call an asynchronous function anonymously by function @func and serial @buffer of size @size
*
*  @param[in] func
*    Reference to function to be called
*
*  @param[in] buffer
*    String representing a map to be deserialized into arguments of the function
*
*  @param[in] size
*    Size of string @buffer
*
*  @param[in] allocator
*    Pointer to allocator will allocate the value
*
*  @param[in] resolve_callback
*    Pointer to function that will be executed when task completion
*      @param[in] void *
*        Value representing the result of the future resolution
*      @param[in] void *
*        A reference to @data that will be used as a closure for the chain
*      @return
*        Value containing the result of the operation,
*        it will be wrapped into a future later on to be returned by the function
*
*  @param[in] reject_callback
*    Pointer to function that will be executed when task error (signature is identical as resolve_callback)
*
*  @param[in] data
*    Pointer to a context that will act as a closure for the chain
*
*  @return
*    Pointer to value containing the result of the call returned by @resolve_callback or @reject_callback wrapped in a future
*/
METACALL_API void *metacallfms_await(void *func, const char *buffer, size_t size, void *allocator, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data);

/**
*  @brief
*    Get the class by @name
*
*  @param[in] name
*    Name of the class
*
*  @return
*    Class reference, null if the class does not exist
*/
METACALL_API void *metacall_class(const char *name);

/**
*  @brief
*    Call a class method anonymously by value array @args (this procedure assumes there's no overloaded methods and does type conversion on values)
*
*  @param[in] cls
*    Pointer to the class
*
*  @param[in] name
*    Name of the method
*
*  @param[in] args
*    Array of pointers to data
*
*  @param[in] size
*    Number of elements of args array
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallv_class(void *cls, const char *name, void *args[], size_t size);

/**
*  @brief
*    Call a class method anonymously by value array @args and return value type @ret (helps to resolve overloading methods)
*
*  @param[in] cls
*    Pointer to the class
*
*  @param[in] name
*    Name of the method
*
*  @param[in] ret
*    Type of the return value of the method
*
*  @param[in] args
*    Array of pointers to data
*
*  @param[in] size
*    Number of elements of args array
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallt_class(void *cls, const char *name, const enum metacall_value_id ret, void *args[], size_t size);

/**
*  @brief
*    Create a new object instance from @cls by value array @args
*
*  @param[in] cls
*    Pointer to the class
*
*  @param[in] name
*    Name of the new object
*
*  @param[in] args
*    Array of pointers constructor parameters
*
*  @param[in] size
*    Number of elements of constructor parameters
*
*  @return
*    Pointer to the new object value instance
*/
METACALL_API void *metacall_class_new(void *cls, const char *name, void *args[], size_t size);

/**
*  @brief
*    Get an attribute from @cls by @key name
*
*  @param[in] cls
*    Pointer to the class
*
*  @param[in] key
*    Name of the attribute to get
*
*  @return
*    Pointer to the class attribute value or NULL if an error occurred
*/
METACALL_API void *metacall_class_static_get(void *cls, const char *key);

/**
*  @brief
*    Set an attribute to @cls by @key name
*
*  @param[in] cls
*    Pointer to the class
*
*  @param[in] key
*    Name of the attribute to set
*
*  @param[in] value
*    Value to set
*
*  @return
*    Non-zero integer if an error ocurred
*/
METACALL_API int metacall_class_static_set(void *cls, const char *key, void *v);

/**
*  @brief
*    Call an object method anonymously by value array @args
*
*  @param[in] obj
*    Pointer to the object
*
*  @param[in] name
*    Name of the method
*
*  @param[in] args
*    Array of pointers to data
*
*  @param[in] size
*    Number of elements of args array
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallv_object(void *obj, const char *name, void *args[], size_t size);

/**
*  @brief
*    Call a object method anonymously by value array @args and return value type @ret (helps to resolve overloading methods)
*
*  @param[in] obj
*    Pointer to the object
*
*  @param[in] name
*    Name of the method
*
*  @param[in] ret
*    Type of the return value of the method
*
*  @param[in] args
*    Array of pointers to data
*
*  @param[in] size
*    Number of elements of args array
*
*  @return
*    Pointer to value containing the result of the call
*/
METACALL_API void *metacallt_object(void *obj, const char *name, const enum metacall_value_id ret, void *args[], size_t size);

/**
*  @brief
*    Get an attribute from @obj by @key name
*
*  @param[in] obj
*    Pointer to the object
*
*  @param[in] key
*    Name of the attribute to get
*
*  @return
*    Pointer to the object attribute value or NULL if an error occurred
*/
METACALL_API void *metacall_object_get(void *obj, const char *key);

/**
*  @brief
*    Set an attribute to @obj by @key name
*
*  @param[in] obj
*    Pointer to the object
*
*  @param[in] key
*    Name of the attribute to set
*
*  @param[in] value
*    Value to set
*
*  @return
*    Non-zero integer if an error ocurred
*/
METACALL_API int metacall_object_set(void *obj, const char *key, void *v);

/**
*  @brief
*    Get the value contained by throwable object @th
*
*  @param[in] th
*    Pointer to the throwable object
*
*  @return
*    Pointer to the value inside of the throwable or NULL in case of error
*/
METACALL_API void *metacall_throwable_value(void *th);

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
METACALL_API char *metacall_inspect(size_t *size, void *allocator);

/**
*  @brief
*    Convert the value @v to serialized string
*
*  @param[in] name
*    Name of the serial to be used
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
METACALL_API char *metacall_serialize(const char *name, void *v, size_t *size, void *allocator);

/**
*  @brief
*    Convert the string @buffer to value
*
*  @param[in] name
*    Name of the serial to be used
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
METACALL_API void *metacall_deserialize(const char *name, const char *buffer, size_t size, void *allocator);

/**
*  @brief
*    Clear handle from memory and unload related resources
*
*  @param[in] handle
*    Reference to the handle to be unloaded
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_clear(void *handle);

/**
*  @brief
*    Get the plugin extension handle to be used for loading plugins
*
*  @return
*    Pointer to the extension handle, or null if it failed to load
*/
METACALL_API void *metacall_plugin_extension(void);

/**
*  @brief
*    Get the plugin extension path to be used for accessing the plugins folder
*
*  @return
*    String containing the core plugin path, or null if it failed to load the plugin extension
*/
METACALL_API const char *metacall_plugin_path(void);

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
*    Provide the module version struct
*
*  @return
*    Static struct containing unpacked version
*/
METACALL_API const struct metacall_version_type *metacall_version(void);

/**
*  @brief
*    Provide the module version hexadecimal value
*    with format 0xMMIIPPPP where M is @major,
*    I is @minor and P is @patch
*
*  @param[in] major
*    Unsigned integer representing major version
*
*  @param[in] minor
*    Unsigned integer representing minor version
*
*  @param[in] patch
*    Unsigned integer representing patch version
*
*  @return
*    Hexadecimal integer containing packed version
*/
METACALL_API uint32_t metacall_version_hex_make(unsigned int major, unsigned int minor, unsigned int patch);

/**
*  @brief
*    Provide the module version hexadecimal value
*    with format 0xMMIIPPPP where M is major,
*    I is minor and P is patch
*
*  @return
*    Hexadecimal integer containing packed version
*/
METACALL_API uint32_t metacall_version_hex(void);

/**
*  @brief
*    Provide the module version string
*
*  @return
*    Static string containing module version
*/
METACALL_API const char *metacall_version_str(void);

/**
*  @brief
*    Provide the module version revision string
*
*  @return
*    Static string containing module version revision
*/
METACALL_API const char *metacall_version_revision(void);

/**
*  @brief
*    Provide the module version name
*
*  @return
*    Static string containing module version name
*/
METACALL_API const char *metacall_version_name(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*/
METACALL_API const char *metacall_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_H */
