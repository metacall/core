/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
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
