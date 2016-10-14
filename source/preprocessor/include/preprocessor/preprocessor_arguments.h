/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_ARGUMENTS_H
#define PREPROCESSOR_ARGUMENTS_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_empty.h>
#include <preprocessor/preprocessor_concatenation.h>
#include <preprocessor/preprocessor_boolean.h>
#include <preprocessor/preprocessor_if.h>
#include <preprocessor/preprocessor_tuple.h>
#include <preprocessor/preprocessor_comma.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define PREPROCESSOR_ARGS_SIZE 64

/* -- Macros -- */

#define PREPROCESSOR_ARGS_COUNT_SEQ_IMPL() \
	63, 62, 61, 60, \
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define PREPROCESSOR_ARGS_COMMA_SEQ_IMPL() \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
	1, 1, 0

#define PREPROCESSOR_ARGS_N_IMPL( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
	_31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
	_41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
	_51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
	_61, _62, _63, \
	N, ...) N

#define PREPROCESSOR_ARGS_FIRST(first, ...) first

#define PREPROCESSOR_ARGS_FIRST_REMOVE(first, ...) __VA_ARGS__

#define PREPROCESSOR_ARGS_SECOND(first, second, ...) second

#define PREPROCESSOR_ARGS_SECOND_REMOVE(first, second, ...) first, __VA_ARGS__

#define PREPROCESSOR_ARGS_FIRST_OR_EMPTY_IMPL(...) \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
		PREPROCESSOR_ARGS_FIRST(__VA_ARGS__, PREPROCESSOR_EMPTY_SYMBOL()), \
		PREPROCESSOR_EMPTY_SYMBOL() \
	)

#define PREPROCESSOR_ARGS_FIRST_OR_EMPTY(...) \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
		PREPROCESSOR_ARGS_FIRST_OR_EMPTY_IMPL(__VA_ARGS__), \
		PREPROCESSOR_EMPTY_SYMBOL() \
	)

#if defined(__GNUC__)
#	define PREPROCESSOR_ARGS_SECOND_OR_EMPTY_IMPL(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
			PREPROCESSOR_ARGS_FIRST(__VA_ARGS__), \
			PREPROCESSOR_EMPTY_SYMBOL() \
		)
#	define PREPROCESSOR_ARGS_SECOND_OR_EMPTY(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_AT_LEAST_TWO(__VA_ARGS__), \
			PREPROCESSOR_ARGS_SECOND_OR_EMPTY_IMPL(PREPROCESSOR_ARGS_FIRST_REMOVE(__VA_ARGS__)), \
			PREPROCESSOR_EMPTY_SYMBOL() \
		)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL_I(First, ...) \
		PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL(...) \
		PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL_I(__VA_ARGS__)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
			PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL(__VA_ARGS__), \
			0 \
		)
#elif defined(_MSC_VER)
#	define PREPROCESSOR_ARGS_SECOND_OR_EMPTY_IMPL(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
			PREPROCESSOR_ARGS_FIRST(__VA_ARGS__, PREPROCESSOR_EMPTY_SYMBOL()), \
			PREPROCESSOR_EMPTY_SYMBOL() \
		)
#	define PREPROCESSOR_ARGS_SECOND_OR_EMPTY(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_AT_LEAST_TWO(__VA_ARGS__), \
			PREPROCESSOR_ARGS_SECOND_OR_EMPTY_IMPL( \
				PREPROCESSOR_ARGS_FIRST_REMOVE(__VA_ARGS__, PREPROCESSOR_EMPTY_SYMBOL()) \
			), \
			PREPROCESSOR_EMPTY_SYMBOL() \
		)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL_I(first, ...) \
		PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL(...) \
		PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL_I(\
			PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
				__VA_ARGS__, \
				PREPROCESSOR_TUPLE_PREPEND(~, __VA_ARGS__) \
			) \
		)
#	define PREPROCESSOR_ARGS_AT_LEAST_TWO(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
			PREPROCESSOR_ARGS_AT_LEAST_TWO_IMPL((__VA_ARGS__)), \
			0 \
		)
#else
#	error "Unkwnown preprocessor implementation for arguments."
#endif

#define PREPROCESSOR_ARGS_NOT_EMPTY_IMPL(...) \
	PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__)

#define PREPROCESSOR_ARGS_NOT_EMPTY(...) \
	PREPROCESSOR_NOT(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__))

#define PREPROCESSOR_ARGS_EMPTY_CASE_IMPL_0001 ,

#define PREPROCESSOR_ARGS_EMPTY_CASE(_0, _1, _2, _3, _4) \
	_0 ## _1 ## _2 ## _3 ## _4

#define PREPROCESSOR_ARGS_EMPTY_IMPL_I(_0, _1, _2, _3) \
	PREPROCESSOR_ARGS_COMMA(PREPROCESSOR_ARGS_EMPTY_CASE(PREPROCESSOR_ARGS_EMPTY_CASE_IMPL_, _0, _1, _2, _3))

#define PREPROCESSOR_ARGS_EMPTY_IMPL(_0, _1, _2, _3) \
	PREPROCESSOR_ARGS_EMPTY_IMPL_I(_0, _1, _2, _3)

#define PREPROCESSOR_ARGS_EMPTY(...) \
	PREPROCESSOR_ARGS_EMPTY_IMPL( \
		PREPROCESSOR_ARGS_COMMA(__VA_ARGS__), \
		PREPROCESSOR_ARGS_COMMA(PREPROCESSOR_COMMA_VARIDIC __VA_ARGS__), \
		PREPROCESSOR_ARGS_COMMA(__VA_ARGS__ ()), \
		PREPROCESSOR_ARGS_COMMA(PREPROCESSOR_COMMA_VARIDIC __VA_ARGS__ ()) \
	)

#if defined(__GNUC__)
#	define PREPROCESSOR_ARGS_COUNT_IMPL(...) \
		PREPROCESSOR_ARG_N_IMPL(__VA_ARGS__)
#	define PREPROCESSOR_ARGS_STRINGIFY_EMPTY_IMPL(...) # __VA_ARGS__
#	define PREPROCESSOR_ARGS_STRINGIFY_EMPTY(...) PREPROCESSOR_ARGS_STRINGIFY_EMPTY_IMPL(__VA_ARGS__)
#	define PREPROCESSOR_ARGS_COUNT(...) \
		(PREPROCESSOR_ARGS_COUNT_IMPL(__VA_ARGS__, PREPROCESSOR_ARGS_COUNT_SEQ_IMPL()) - \
		(sizeof(PREPROCESSOR_ARGS_STRINGIFY_EMPTY(__VA_ARGS__)) == 1))
#	define PREPROCESSOR_ARGS_COMMA(...) \
		PREPROCESSOR_ARGS_COUNT_IMPL(__VA_ARGS__, PREPROCESSOR_ARGS_COMMA_SEQ_IMPL())
#elif defined(_MSC_VER)
#	define PREPROCESSOR_ARGS_COUNT_PREFIX__PREPROCESSOR_ARGS_COUNT_POSTFIX ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,0
#	define PREPROCESSOR_ARGS_COUNT_IMPL(expr) PREPROCESSOR_ARG_N_IMPL expr
#	define PREPROCESSOR_ARGS_COUNT(...) \
		PREPROCESSOR_ARGS_COUNT_IMPL((PREPROCESSOR_ARGS_COUNT_PREFIX_ ## __VA_ARGS__ ## _PREPROCESSOR_ARGS_COUNT_POSTFIX, PREPROCESSOR_ARGS_COUNT_SEQ_IMPL()))
#	define PREPROCESSOR_ARGS_COMMA(...) \
		PREPROCESSOR_ARGS_COUNT_IMPL((PREPROCESSOR_ARGS_COUNT_PREFIX_ ## __VA_ARGS__ ## _PREPROCESSOR_ARGS_COUNT_POSTFIX, PREPROCESSOR_ARGS_COMMA_SEQ_IMPL()))
#else
#	error "Unkwnown preprocessor implementation for arguments."
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_ARGUMENTS_H */
