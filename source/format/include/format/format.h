/*
 *	Format Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

#ifndef FORMAT_H
#define FORMAT_H 1

/* -- Headers -- */

#include <format/format_api.h>

#include <format/format_specifier.h>
#include <format/format_print.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

FORMAT_API const char * format_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* FORMAT_H */
