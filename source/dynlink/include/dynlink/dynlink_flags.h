/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_FLAGS_H
#define DYNLINK_FLAGS_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_FLAGS_BIND_NOW		(0x01 << 0x00)			/**< Inmediate loading bind flag */
#define DYNLINK_FLAGS_BIND_LAZY		(0x01 << 0x01)			/**< Lazy loading bind flag */
#define DYNLINK_FLAGS_BIND_LOCAL	(0x01 << 0x02)			/**< Private visibility bind flag */
#define DYNLINK_FLAGS_BIND_GLOBAL	(0x01 << 0x03)			/**< Public visibility bind flag */

/* -- Macros -- */

/**
*  @brief
*    Assign to the left operator the right literal
*
*  Using flag as a 0 resets the flags. Combined flags can
*  be passed to the macro and they will be assigned correctly too.
*  It can be used for generic and implementation flags.
*
*  DYNLINK_FLAGS_SET(flags, 0);
*  DYNLINK_FLAGS_SET(flags, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
*  DYNLINK_FLAGS_SET(flags_impl, RTLD_LAZY | RTLD_GLOBAL);
*
*  @param[out] flags
*    The left operator that will be overwritten
*
*  @param[in] flag
*    The right literal input parameter
*
*/
#define DYNLINK_FLAGS_SET(flags, flag) \
	do \
	{ \
		flags = (flag); \
	} \
	while (0)

/**
*  @brief
*    Append to the left operator the right literal
*
*  Using flag as a 0 does nothing. Combined flags can
*  be passed to the macro and they will be added correctly too.
*  It can be used for generic and implementation flags.
*
*  DYNLINK_FLAGS_ADD(nothing, 0); 
*  DYNLINK_FLAGS_ADD(flags, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
*  DYNLINK_FLAGS_ADD(flags_impl, RTLD_LAZY | RTLD_GLOBAL);
*
*  @param[out] flags
*    The left operator that will be added
*
*  @param[in] flag
*    The literal input parameter
*
*/
#define DYNLINK_FLAGS_ADD(flags, flag) \
	do \
	{ \
		flags |= (flag); \
	} \
	while (0)

/**
*  @brief
*    Check flags operator
*
*  This macro must be used as a conditional guard.
*  Using flag as a 0 never enters to the guard.
*  Combined flags can be passed to the macro.
*  It can be used for generic and implementation flags.
*
*  if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LAZY))
*  {
*    ...
*  }
*
*  if (DYNLINK_FLAGS_CHECK(flags_impl, RTLD_LAZY | RTLD_GLOBAL))
*  {
*    ...
*  }
*
*  @param[out] flags
*    The left operator that will be check against the literal
*
*  @param[in] flag
*    The literal input parameter
*
*/
#define DYNLINK_FLAGS_CHECK(flags, flag) \
		(((flags) & (flag)) != 0)

/* -- Type definitions -- */

typedef int							dynlink_flags;			/**< Dynamically linked shared object flags */

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_FLAGS_H */
