/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_FUNCTION_H
#define REFLECT_FUNCTION_H 1

#include <reflect/reflect_async.h>
#include <reflect/reflect_signature.h>
#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

struct function_type;

typedef void *function_impl;

typedef struct function_type *function;

typedef void *function_args[];

typedef value function_return;

typedef value (*function_resolve_callback)(value, void *);

typedef value (*function_reject_callback)(value, void *);

typedef int (*function_impl_interface_create)(function, function_impl);

typedef function_return (*function_impl_interface_invoke)(function, function_impl, function_args, size_t);

typedef function_return (*function_impl_interface_await)(function, function_impl, function_args, size_t, function_resolve_callback, function_reject_callback, void *);

typedef void (*function_impl_interface_destroy)(function, function_impl);

typedef struct function_interface_type
{
	function_impl_interface_create create;
	function_impl_interface_invoke invoke;
	function_impl_interface_await await;
	function_impl_interface_destroy destroy;

} * function_interface;

typedef function_interface (*function_impl_interface_singleton)(void);

/**
*  @brief
*    Create a new function with the given name and argument count
*
*  @param[in] name
*    Name identifier for the function
*  @param[in] args_count
*    Number of arguments the function accepts
*  @param[in] impl
*    Pointer to the language-specific function implementation
*  @param[in] singleton
*    Singleton accessor for the function interface (create, invoke, await, destroy)
*
*  @return
*    Pointer to the newly created function on success, NULL on failure
*/
REFLECT_API function function_create(const char *name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton);

/**
*  @brief
*    Increment the reference count of a function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int function_increment_reference(function func);

/**
*  @brief
*    Decrement the reference count of a function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int function_decrement_reference(function func);

/**
*  @brief
*    Set the async behavior of a function
*
*  @param[in] func
*    Pointer to the function
*  @param[in] async
*    Async identifier specifying the async behavior
*/
REFLECT_API void function_async(function func, enum async_id async);

/**
*  @brief
*    Get the async identifier of a function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Async identifier of the function
*/
REFLECT_API enum async_id function_async_id(function func);

/**
*  @brief
*    Bind closure data to a function
*
*  @param[in] func
*    Pointer to the function
*  @param[in] data
*    Opaque pointer to closure data
*/
REFLECT_API void function_bind(function func, void *data);

/**
*  @brief
*    Get the closure data bound to a function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Opaque pointer to the closure data, or NULL if none is bound
*/
REFLECT_API void *function_closure(function func);

/**
*  @brief
*    Get the name of a function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    String containing the function name, or NULL if func is NULL
*    or no name was provided at creation time
*/
REFLECT_API const char *function_name(function func);

/**
*  @brief
*    Get the signature of a function (argument types and return type)
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Pointer to the function's signature
*/
REFLECT_API signature function_signature(function func);

/**
*  @brief
*    Generate a metadata representation of the function
*
*  @param[in] func
*    Pointer to the function
*
*  @return
*    Value containing the metadata map, or NULL on failure
*/
REFLECT_API value function_metadata(function func);

/**
*  @brief
*    Invoke a function synchronously with the given arguments
*
*  @param[in] func
*    Pointer to the function to call
*  @param[in] args
*    Array of value arguments to pass to the function
*  @param[in] size
*    Number of arguments in the args array
*
*  @return
*    Return value of the function call, or NULL on failure
*/
REFLECT_API function_return function_call(function func, function_args args, size_t size);

/**
*  @brief
*    Invoke a function asynchronously with resolve and reject callbacks
*
*  @param[in] func
*    Pointer to the function to call
*  @param[in] args
*    Array of value arguments to pass to the function
*  @param[in] size
*    Number of arguments in the args array
*  @param[in] resolve_callback
*    Callback invoked when the async operation completes successfully
*  @param[in] reject_callback
*    Callback invoked when the async operation fails
*  @param[in] context
*    Opaque pointer passed to both callbacks
*
*  @return
*    Return value (typically a future), or NULL on failure
*/
REFLECT_API function_return function_await(function func, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context);

/**
*  @brief
*    Print debug statistics about function allocations and usage
*
*  @note
*    Only produces output when the memory tracker is enabled
*    (OPTION_MEMORY_TRACKER=ON at build time)
*/
REFLECT_API void function_stats_debug(void);

/**
*  @brief
*    Destroy a function and free all associated resources.
*    Passing NULL is safe and has no effect.
*
*  @param[in] func
*    Pointer to the function to destroy
*/
REFLECT_API void function_destroy(function func);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_FUNCTION_H */
