/*
 *	Configuration Library by Parra Studios
 *	A cross-platform library for managing multiple configuration formats.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef CONFIGURATION_OBJECT_HANDLE_H
#define CONFIGURATION_OBJECT_HANDLE_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct configuration_type;

/* -- Type Definitions -- */

typedef struct configuration_type *configuration;

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_OBJECT_HANDLE_H */
