/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
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

#ifndef portability_static_assert
	#if defined(__GNUC__) || defined(__clang__)
		#define portability_static_assert_impl_expr(predicate, expr) \
			typedef __attribute__((unused)) char expr[2 * !!(predicate)-1]
	#else
		#define portability_static_assert_impl_expr(predicate, expr) \
			typedef char expr[2 * !!(predicate)-1]
	#endif

	#if defined(__COUNTER__)
		#define portability_static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __COUNTER__))
	#elif defined(__LINE__)
		/* WARNING: It can collide if it's used in header files */
		#define portability_static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __LINE__))
	#else
		#define portability_static_assert_impl_line(macro, predicate, expr) macro((predicate), expr)
	#endif

	#define portability_static_assert_impl(macro, predicate) portability_static_assert_impl_line(macro, predicate, portability_static_assert_)

	#define portability_static_assert(predicate, message) portability_static_assert_impl(portability_static_assert_impl_expr, predicate)
#endif

#if defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	#include <sanitizer/common_interface_defs.h>
	#define portability_assert(condition) \
		do \
		{ \
			if (!(condition)) \
			{ \
				__sanitizer_print_stack_trace(); \
				assert(condition); \
			} \
		} while (0)
#else
	#define portability_assert(condition) assert(condition)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_ASSERT_H */
