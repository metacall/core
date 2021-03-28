/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <metacall/metacall_version.h>

#include <adt/adt.h>

const char *adt_print_info()
{
	static const char adt_info[] =
		"Abstract Data Type Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef ADT_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return adt_info;
}
