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

#ifndef PORTABILITY_DEPENDENCY_H
#define PORTABILITY_DEPENDENCY_H 1

/* -- Headers -- */

#include <portability/portability_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef int (*portability_dependendency_iterate_cb)(const char *library, void *data);

/* -- Methods -- */

int portability_dependendency_iterate(portability_dependendency_iterate_cb callback, void *data);

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_DEPENDENCY_H */
