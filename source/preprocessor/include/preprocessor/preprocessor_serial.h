/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_SERIAL_H
#define PREPROCESSOR_SERIAL_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define PREPROCESSOR_SERIAL_ID_IMPL(expr) \
	PREPROCESSOR_CONCAT(expr, __LINE__)

#define PREPROCESSOR_SERIAL_ID(expr) \
	PREPROCESSOR_SERIAL_ID_IMPL(expr)

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_SERIAL_H */
