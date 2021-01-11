/*
 *	Format Library by Parra Studios
 *	A cross-platform library for supporting formatted input / output.
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

#include <format/format_print.h>

/* -- Methods -- */

int snprintf_impl_c89(char * s, size_t n, const char * format, ...)
{
	va_list args;

	int result;

	/* TODO: Implement stack smashing prevention */
	(void)n;

	va_start(args, format);

	result = vsprintf(s, format, args);

	va_end(args);

	return result;
}

int vsnprintf_impl_c89(char * s, size_t n, const char * format, va_list arg)
{
	/* TODO: Implement stack smashing prevention */
	(void)n;

	return vsprintf(s, format, arg);
}
