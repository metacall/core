/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_IF_H
#define PREPROCESSOR_IF_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

#if defined(_MSC_VER)
#	define PREPROCESSOR_IIF_IMPL_I(expr) expr
#	define PREPROCESSOR_IIF_IMPL(value, true_block, false_block) PREPROCESSOR_IIF_IMPL_I(PREPROCESSOR_IIF_IMPL_ ## value(true_block, false_block))
#else
#	define PREPROCESSOR_IIF_IMPL(value, true_block, false_block) PREPROCESSOR_IIF_IMPL_ ## value(true_block, false_block)
#endif

#if defined(__MWERKS__)
#	define PREPROCESSOR_IIF_EXPAND(expr) PREPROCESSOR_IIF_IMPL ## expr
#	define PREPROCESSOR_IIF(value, true_block, false_block) PREPROCESSOR_IIF_EXPAND((value, true_block, false_block))
#else
#	define PREPROCESSOR_IIF(value, true_block, false_block) PREPROCESSOR_IIF_IMPL(value, true_block, false_block)
#endif

#if defined(__EDG__) || defined(__EDG_VERSION__)
#	define PREPROCESSOR_IF_IMPL(condition, true_block, false_block) PREPROCESSOR_IIF(PREPROCESSOR_BOOL(condition), true_block, false_block)
#	define PREPROCESSOR_IF(condition, true_block, false_block) PREPROCESSOR_IF_IMPL(condition, true_block, false_block)
#else
#	define PREPROCESSOR_IF(condition, true_block, false_block) PREPROCESSOR_IIF(PREPROCESSOR_BOOL(condition), true_block, false_block)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_IF_H */
