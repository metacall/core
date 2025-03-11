/*
 *	Preprocessor Library by Parra Studios
 *	A generic header-only preprocessor metaprogramming library.
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

#ifndef PREPROCESSOR_TUPLE_H
#define PREPROCESSOR_TUPLE_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define PREPROCESSOR_TUPLE_EXPAND_MACRO(macro, ...) \
	macro __VA_ARGS__

#define PREPROCESSOR_TUPLE_EXPAND(...) \
	PREPROCESSOR_TUPLE_EXPAND_IMPL __VA_ARGS__

#define PREPROCESSOR_TUPLE_MAKE(...) \
	(PREPROCESSOR_TUPLE_EXPAND_IMPL(__VA_ARGS__))

#define PREPROCESSOR_TUPLE_EXPAND_IMPL(...) __VA_ARGS__

#define PREPROCESSOR_TUPLE_PREPEND(expr, tuple) \
	PREPROCESSOR_TUPLE_MAKE(expr, PREPROCESSOR_TUPLE_EXPAND(tuple))

#define PREPROCESSOR_TUPLE_APPEND(expr, tuple) \
	PREPROCESSOR_TUPLE_MAKE(PREPROCESSOR_TUPLE_EXPAND(tuple), expr)

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_TUPLE_H */
