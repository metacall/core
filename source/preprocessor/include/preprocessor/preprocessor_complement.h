/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_COMPLEMENT_H
#define PREPROCESSOR_COMPLEMENT_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define PREPROCESSOR_COMPL_IMPL_0 1
#define PREPROCESSOR_COMPL_IMPL_1 0

/* -- Macros -- */

#if defined(_WIN32) && defined(_MSC_VER)
#	define PREPROCESSOR_COMPL_IMPL_EXPAND(expr) expr
#	define PREPROCESSOR_COMPL_IMPL(expr) PREPROCESSOR_COMPL_IMPL_EXPAND(PREPROCESSOR_COMPL_IMPL_ ## expr)
#else
#	define PREPROCESSOR_COMPL_IMPL(expr) PREPROCESSOR_COMPL_IMPL_ ## expr
#endif

#if defined(__MWERKS__)
#	define PREPROCESSOR_COMPL_TOKEN(expr) PREPROCESSOR_COMPL_IMPL ## expr
#	define PREPROCESSOR_COMPL(expr) PREPROCESSOR_COMPL_TOKEN((expr))
#else
#	define PREPROCESSOR_COMPL(expr) PREPROCESSOR_COMPL_IMPL(expr)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_COMPLEMENT_H */
