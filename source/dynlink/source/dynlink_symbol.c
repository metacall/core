/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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
