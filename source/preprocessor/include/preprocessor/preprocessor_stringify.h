/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#define PREPROCESSOR_STRINGIFY_IMPL(expr) # expr

#define PREPROCESSOR_STRINGIFY(expr) PREPROCESSOR_STRINGIFY_IMPL(expr)

#define PREPROCESSOR_STRINGIFY_VARIDIC(...) \
	PREPROCESSOR_FOR_EACH(PREPROCESSOR_STRINGIFY, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_STRINGIFY_H */
