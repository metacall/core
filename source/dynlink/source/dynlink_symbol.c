/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

/* -- Headers -- */

#include <dynlink/dynlink_symbol.h>
#include <dynlink/dynlink_interface.h>

#include <string.h>

/* -- Methods -- */

int dynlink_symbol_name_mangle(dynlink_symbol_name symbol_name, dynlink_symbol_name_man symbol_mangled)
{
	const char symbol_prefix[] = DYNLINK_SYMBOL_PREFIX_STR();

	size_t symbol_prefix_length = sizeof(symbol_prefix) - 1;

	if (symbol_prefix_length > 0)
	{
		memcpy(symbol_mangled, symbol_prefix, symbol_prefix_length);
	}

	strcpy(&symbol_mangled[symbol_prefix_length], symbol_name);

	return 0;
}
