/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PORTABILITY_ATEXIT_H
#define PORTABILITY_ATEXIT_H 1

/* -- Headers -- */

#include <portability/portability_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef void (*portability_atexit_fn)(void);

/* -- Methods -- */

/**
*  @brief
*    Initialize atexit instance for custom at exit handlers
*
*  @return
*    Zero if success, different from zero otherwise
*/
PORTABILITY_API int portability_atexit_initialize(void);

/**
*  @brief
*    Register handler to be run at exit
*
*  @param[in] handler
*    Function pointer to the handler that will be executed at exit
*
*  @return
*    Zero if success, different from zero otherwise
*/
PORTABILITY_API int portability_atexit_register(portability_atexit_fn handler);

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_ATEXIT_H */
