/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_H
#define REFLECT_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_signature.h>
#include <reflect/reflect_function.h>

#ifdef __cplusplus
extern "C" {
#endif

REFLECT_API const char * reflect_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_H */
