/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_CONCATENATION_H
#define PREPROCESSOR_CONCATENATION_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#if defined(_MSC_VER)
#	define PREPROCESSOR_CONCAT_IMPL_II(token, expr) expr
#	define PREPROCESSOR_CONCAT_IMPL_I(left, right) PREPROCESSOR_CONCAT_IMPL_II(~, left ## right)
#	define PREPROCESSOR_CONCAT_IMPL(left, right) PREPROCESSOR_CONCAT_IMPL_I(left, right)
#else
#	define PREPROCESSOR_CONCAT_IMPL(left, right) left ## right
#endif

#if defined(__MWERKS__)
#	define PREPROCESSOR_CONCAT_IMPL_TOKEN(token) PREPROCESSOR_CONCAT_IMPL ## token
#	define PREPROCESSOR_CONCAT(left, right) PREPROCESSOR_CONCAT_IMPL_TOKEN((left, right))
#else
#	define PREPROCESSOR_CONCAT(left, right) PREPROCESSOR_CONCAT_IMPL(left, right)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_CONCATENATION_H */
