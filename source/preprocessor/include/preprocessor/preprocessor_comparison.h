/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_COMPARISON_H
#define PREPROCESSOR_COMPARISON_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_bit.h>
#include <preprocessor/preprocessor_concatenation.h>
#include <preprocessor/preprocessor_detection.h>
#include <preprocessor/preprocessor_empty.h>
#include <preprocessor/preprocessor_if.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define PREPROCESSOR_COMPARE_IMPL(left, right) \
	PREPROCESSOR_DETECT_PARENTHESIS(           \
		PREPROCESSOR_COMPARE_##left(PREPROCESSOR_COMPARE_##right)(()))

#define PREPROCESSOR_NOT_EQUAL(left, right)                                                                \
	PREPROCESSOR_IF_IMPL(                                                                                  \
		PREPROCESSOR_BIT_AND(PREPROCESSOR_DETECT_COMPARABLE(left))(PREPROCESSOR_DETECT_COMPARABLE(right)), \
		PREPROCESSOR_COMPARE_IMPL,                                                                         \
		1 PREPROCESSOR_EMPTY_EXPANSION_VARIADIC)                                                           \
	(left, right)

#define PREPROCESSOR_EQUAL(left, right) \
	PREPROCESSOR_COMPL(PREPROCESSOR_NOT_EQUAL(left, right))

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_COMPARISON_H */
