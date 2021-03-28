/*
 *	Environment Library by Parra Studios
 *	A cross-platform library for supporting platform specific environment features.
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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H 1

/* -- Headers -- */

#include <environment/environment_api.h>

#include <environment/environment_variable.h>
#include <environment/environment_variable_path.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

ENVIRONMENT_API const char *environment_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H */
