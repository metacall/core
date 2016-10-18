/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header-only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_arithmetic.h>
#include <preprocessor/preprocessor_arguments.h>
#include <preprocessor/preprocessor_bit.h>
#include <preprocessor/preprocessor_boolean.h>
#include <preprocessor/preprocessor_comma.h>
#include <preprocessor/preprocessor_comparison.h>
#include <preprocessor/preprocessor_complement.h>
#include <preprocessor/preprocessor_concatenation.h>
#include <preprocessor/preprocessor_detection.h>
#include <preprocessor/preprocessor_empty.h>
#include <preprocessor/preprocessor_for.h>
#include <preprocessor/preprocessor_if.h>
#include <preprocessor/preprocessor_serial.h>
#include <preprocessor/preprocessor_stringify.h>
#include <preprocessor/preprocessor_tuple.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

PREPROCESSOR_API const char * preprocessor_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_H */
