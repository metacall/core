/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
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

#ifndef PORTABILITY_ASSERT_H
#define PORTABILITY_ASSERT_H 1

/* -- Headers -- */

#include <portability/portability_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <assert.h>

#include <preprocessor/preprocessor_concatenation.h>

/* -- Macros -- */

#ifndef static_assert
#	define static_assert_impl_expr(predicate, expr) \
		typedef char expr[2 * !!(predicate) - 1]

#	if defined(__COUNTER__)
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __COUNTER__))
#	elif defined(__LINE__)
		/* WARNING: It can collide if it's used in header files */
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __LINE__))
#	else
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), expr)
#	endif

#	define static_assert_impl(macro, predicate) static_assert_impl_line(macro, predicate, static_assert_)

#	define static_assert(predicate, message) static_assert_impl(static_assert_impl_expr, predicate)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_ASSERT_H */
