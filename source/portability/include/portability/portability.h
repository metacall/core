/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
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

#ifndef PORTABILITY_H
#define PORTABILITY_H 1

/* -- Headers -- */

#include <portability/portability_api.h>

#include <portability/portability_assert.h>
#include <portability/portability_executable_path.h>
#include <portability/portability_library_path.h>
#include <portability/portability_path.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

PORTABILITY_API const char *portability_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_H */
