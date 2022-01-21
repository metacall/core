/*
 *	Preprocessor Library by Parra Studios
 *	A generic header-only preprocessor metaprogramming library.
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

#ifndef PREPROCESSOR_COMMA_H
#define PREPROCESSOR_COMMA_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_empty.h>
#include <preprocessor/preprocessor_if.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros-- */

#define PREPROCESSOR_COMMA_SYMBOL() ,

#define PREPROCESSOR_COMMA_IF_IMPL(expr) \
	PREPROCESSOR_IF(expr, PREPROCESSOR_COMMA_SYMBOL, PREPROCESSOR_EMPTY_SYMBOL)

#define PREPROCESSOR_COMMA_IF(expr) \
	PREPROCESSOR_COMMA_IF_IMPL(expr)

#define PREPROCESSOR_COMMA_VARIADIC(...) ,

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_COMMA_H */
