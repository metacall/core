/*
 *	Preprocssor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header only preprocessor metaprogramming library.
 *
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

PREPROCESSOR_API const char * preprocessor_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_H */
