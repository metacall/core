/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Macros -- */

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
	#define PREPROCESSOR_IIF_IMPL_EXPAND_ARGS(...) __VA_ARGS__
	#define PREPROCESSOR_IIF_IMPL_EXPAND(expr) expr
	#define PREPROCESSOR_IIF_IMPL_EXPAND_REMOVE_PARENTHESIS(expr) PREPROCESSOR_IIF_IMPL_EXPAND(PREPROCESSOR_IIF_IMPL_EXPAND_ARGS expr)

	#define PREPROCESSOR_IIF_IMPL_0(true_expr, false_expr) PREPROCESSOR_IIF_IMPL_EXPAND_REMOVE_PARENTHESIS(false_expr)
	#define PREPROCESSOR_IIF_IMPL_1(true_expr, false_expr) PREPROCESSOR_IIF_IMPL_EXPAND_REMOVE_PARENTHESIS(true_expr)
#else
	#define PREPROCESSOR_IIF_IMPL_0(true_expr, false_expr) false_expr
	#define PREPROCESSOR_IIF_IMPL_1(true_expr, false_expr) true_expr
#endif

#if defined(_MSC_VER) && !defined(__clang__)
	#define PREPROCESSOR_IIF_IMPL_I(expr)						expr
	#define PREPROCESSOR_IIF_IMPL(value, true_expr, false_expr) PREPROCESSOR_IIF_IMPL_I(PREPROCESSOR_IIF_IMPL_##value(true_expr, false_expr))
#else
	#define PREPROCESSOR_IIF_IMPL(value, true_expr, false_expr) PREPROCESSOR_IIF_IMPL_##value(true_expr, false_expr)
#endif

#if defined(__MWERKS__)
	#define PREPROCESSOR_IIF_EXPAND(expr)				   PREPROCESSOR_IIF_IMPL##expr
	#define PREPROCESSOR_IIF(value, true_expr, false_expr) PREPROCESSOR_IIF_EXPAND((value, true_expr, false_expr))
#else
	#define PREPROCESSOR_IIF(value, true_expr, false_expr) PREPROCESSOR_IIF_IMPL(value, true_expr, false_expr)
#endif

#if defined(__EDG__) || defined(__EDG_VERSION__)
	#define PREPROCESSOR_IF_IMPL(condition, true_expr, false_expr) PREPROCESSOR_IIF(PREPROCESSOR_BOOL(condition), true_expr, false_expr)
	#define PREPROCESSOR_IF(condition, true_expr, false_expr)	   PREPROCESSOR_IF_IMPL(condition, true_expr, false_expr)
#else
	#define PREPROCESSOR_IF(condition, true_expr, false_expr) PREPROCESSOR_IIF(PREPROCESSOR_BOOL(condition), (true_expr), (false_expr))
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_IF_H */
