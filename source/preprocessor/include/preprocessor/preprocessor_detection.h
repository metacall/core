/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_DETECTION_H
#define PREPROCESSOR_DETECTION_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_arguments.h>
#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define PREPROCESSOR_DETECT(...) \
	PREPROCESSOR_ARGS_SECOND(__VA_ARGS__, 0, )

#define PREPROCESSOR_DETECT_TOKEN(token) token, 1

#define PREPROCESSOR_DETECT_PARENTHESIS_IMPL(...) \
	PREPROCESSOR_DETECT_TOKEN(~)

#define PREPROCESSOR_DETECT_PARENTHESIS(expr) \
	PREPROCESSOR_DETECT(PREPROCESSOR_DETECT_PARENTHESIS_IMPL expr)

#define PREPROCESSOR_DETECT_COMPARABLE(expr) \
	PREPROCESSOR_DETECT_PARENTHESIS(PREPROCESSOR_CONCAT(PREPROCESSOR_COMPARE_, expr)(()))

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_DETECTION_H */
