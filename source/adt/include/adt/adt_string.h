/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#ifndef ADT_STRING_H
#define ADT_STRING_H 1

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>
#include <string.h>

/* -- Macros -- */

/* This is a temporary solution for safe strings, it can be improved in the future */
#define string_copy(dest, src, dest_size) \
	do \
	{ \
		size_t __string_copy_length = strnlen(src, dest_size - 1); \
		memcpy(dest, src, __string_copy_length); \
		dest[__string_copy_length] = '\0'; \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* ADT_STRING_H */
