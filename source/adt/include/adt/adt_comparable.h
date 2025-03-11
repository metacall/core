/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#ifndef ADT_COMPARABLE_H
#define ADT_COMPARABLE_H 1

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *comparable;

typedef int (*comparable_callback)(const comparable, const comparable);

ADT_API int comparable_callback_str(const comparable a, const comparable b);

ADT_API int comparable_callback_int(const comparable a, const comparable b);

ADT_API int comparable_callback_ptr(const comparable a, const comparable b);

#ifdef __cplusplus
}
#endif

#endif /* ADT_COMPARABLE_H */
