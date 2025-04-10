/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#ifndef METACALL_LINK_H
#define METACALL_LINK_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize link detours and allocate shared memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_initialize(void);

/**
*  @brief
*    Register a function pointer in order to allow function
*    interposition when loading a library, if you register a
*    function @symbol called 'foo', when you try to dlsym (or the equivalent
*    on every platform), you will get the pointer to @fn, even if
*    the symbol does not exist in the library, it will work.
*    Function interposition is required in order to hook into runtimes
*    and dynamically interpose our functions.
*
*  @param[in] tag
*    Name of the loader which the @library belongs to
*
*  @param[in] library
*    Name of the library that is going to be hooked
*
*  @param[in] symbol
*    Name of the function to be interposed
*
*  @param[in] fn
*    Function pointer that will be returned by dlsym (or equivalent) when accessing to @symbol
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_register(const char *tag, const char *library, const char *symbol, void (*fn)(void));

/**
*  @brief
*    Register a function pointer in order to allow function
*    interposition when loading a library, if you register a
*    function @symbol called 'foo', when you try to dlsym (or the equivalent
*    on every platform), you will get the pointer to @fn, even if
*    the symbol does not exist in the library, it will work.
*    Function interposition is required in order to hook into runtimes
*    and dynamically interpose our functions.
*
*  @param[in] loader
*    Pointer to the loader which the @library belongs to
*
*  @param[in] library
*    Name of the library that is going to be hooked
*
*  @param[in] symbol
*    Name of the function to be interposed
*
*  @param[in] fn
*    Function pointer that will be returned by dlsym (or equivalent) when accessing to @symbol
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_register_impl(void *loader, const char *library, const char *symbol, void (*fn)(void));

/**
*  @brief
*    Remove the hook previously registered
*
*  @param[in] tag
*    Name of the loader which the @library belongs to
*
*  @param[in] library
*    Name of the library that is going to be hooked
*
*  @param[in] symbol
*    Name of the function to be interposed
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_unregister(const char *tag, const char *library, const char *symbol);

/**
*  @brief
*    Unregister link detours and destroy shared memory
*/
METACALL_API void metacall_link_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_LINK_H */
