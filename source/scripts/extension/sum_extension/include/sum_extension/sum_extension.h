/*
 *	Extension Library by Parra Studios
 *	An extension for sum numbers.
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

#ifndef SUM_EXTENSION_H
#define SUM_EXTENSION_H 1

#include <sum_extension/sum_extension_api.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

SUM_EXTENSION_API int sum_extension(void *loader, void *handle, void *context);

DYNLINK_SYMBOL_EXPORT(sum_extension);

#ifdef __cplusplus
}
#endif

#endif /* SUM_EXTENSION_H */
