/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_STRINGIFY_H
#define PREPROCESSOR_STRINGIFY_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_for.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

/* Stringify a symbol */
#define PREPROCESSOR_STRINGIFY_IMPL(...) #__VA_ARGS__

#if defined(__MWERKS__)
	#define PREPROCESSOR_STRINGIFY_IMPL_I(tuple) PREPROCESSOR_STRINGIFY_IMPL##tuple
	#define PREPROCESSOR_STRINGIFY(expr)		 PREPROCESSOR_STRINGIFY_IMPL_I((expr))
#elif (defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)) && !defined(__clang__)
	#define PREPROCESSOR_STRINGIFY_IMPL_I(tuple) PREPROCESSOR_STRINGIFY_IMPL tuple
	#define PREPROCESSOR_STRINGIFY(expr)		 PREPROCESSOR_STRINGIFY_IMPL_I((expr))
#else
	#define PREPROCESSOR_STRINGIFY(expr) PREPROCESSOR_STRINGIFY_IMPL(expr)
#endif

/* Stringify a symbol or return empty string if the expression expands to nothing */
#if (defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)) && !defined(__clang__)
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_II(...) #__VA_ARGS__
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL(...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_NOT_EMPTY(__VA_ARGS__), \
			PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_II(__VA_ARGS__), \
			"")
#else
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL(...) #__VA_ARGS__
#endif

#if defined(__MWERKS__)
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_I(tuple) PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL##tuple
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY(expr)		 PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_I((expr))
#elif (defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)) && !defined(__clang__)
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_I(tuple) PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL tuple
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY(expr)		 PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL_I((expr))
#else
	#define PREPROCESSOR_STRINGIFY_OR_EMPTY(expr) PREPROCESSOR_STRINGIFY_OR_EMPTY_IMPL(expr)
#endif

/* Stringify a list of symbols */
#define PREPROCESSOR_STRINGIFY_VARIADIC(...) \
	PREPROCESSOR_FOR_EACH(PREPROCESSOR_STRINGIFY, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_STRINGIFY_H */
