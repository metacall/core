/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_BIT_H
#define PREPROCESSOR_BIT_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_complement.h>
#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

#define PREPROCESSOR_BIT_AND_IMPL_0(expr) 0
#define PREPROCESSOR_BIT_AND_IMPL_1(expr) expr

#define PREPROCESSOR_BIT_OR_IMPL_0(expr) expr
#define PREPROCESSOR_BIT_OR_IMPL_1(expr) 1

#define PREPROCESSOR_BIT_XOR_IMPL_0(expr) expr
#define PREPROCESSOR_BIT_XOR_IMPL_1(expr) PREPROCESSOR_COMPL(expr)

#define PREPROCESSOR_BIT_AND(expr) PREPROCESSOR_CONCAT(PREPROCESSOR_BIT_AND_IMPL_, expr)

#define PREPROCESSOR_BIT_OR(expr) PREPROCESSOR_CONCAT(PREPROCESSOR_BIT_OR_IMPL_, expr)

#define PREPROCESSOR_BIT_XOR(expr) PREPROCESSOR_CONCAT(PREPROCESSOR_BIT_XOR_IMPL_, expr)

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_BIT_H */
