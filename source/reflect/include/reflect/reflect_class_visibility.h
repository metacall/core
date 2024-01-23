/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_CLASS_VISIBILITY_H
#define REFLECT_CLASS_VISIBILITY_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_value.h>

#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLASS_VISIBILITY_X(X) \
	X(PUBLIC, "public", 0), \
		X(PROTECTED, "protected", 1), \
		X(PRIVATE, "private", 2)

#define CLASS_VISIBILITY_ENUM(name, str, id) \
	PREPROCESSOR_CONCAT(VISIBILITY_, name) = id

enum class_visibility_id
{
	CLASS_VISIBILITY_X(CLASS_VISIBILITY_ENUM)
};

REFLECT_API const char *class_visibility_string(enum class_visibility_id visibility);

REFLECT_API value class_visibility_value(enum class_visibility_id visibility);

REFLECT_API value class_visibility_value_pair(enum class_visibility_id visibility);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_CLASS_VISIBILITY_H */
