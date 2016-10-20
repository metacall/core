/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
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
