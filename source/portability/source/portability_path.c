/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_path.h>

#include <string.h>

/* Define separator checking for any platform */
#define PORTABILITY_PATH_SEPARATOR_ALL(chr) (chr == '\\' || chr == '/')

size_t portability_path_get_name(const char *path, size_t path_size, char *name, size_t name_size)
{
	if (path == NULL || name == NULL)
	{
		return 0;
	}

	size_t i, count, last;

	for (i = 0, count = 0, last = 0; path[i] != '\0' && i < path_size && count < name_size; ++i)
	{
		name[count++] = path[i];

		if (PORTABILITY_PATH_SEPARATOR(path[i]))
		{
			count = 0;
		}
		else if (path[i] == '.')
		{
			if (i > 0 && path[i - 1] == '.')
			{
				last = 0;
				count = 0;
			}
			else
			{
				if (count > 0)
				{
					last = count - 1;
				}
				else
				{
					last = 0;
				}
			}
		}
	}

	if (last == 0 && count > 1)
	{
		last = count;
	}

	name[last] = '\0';

	return last + 1;
}

size_t portability_path_get_name_canonical(const char *path, size_t path_size, char *name, size_t name_size)
{
	if (path == NULL || name == NULL)
	{
		return 0;
	}

	size_t i, count, last;

	for (i = 0, count = 0, last = 0; path[i] != '\0' && i < path_size && count < name_size; ++i)
	{
		name[count++] = path[i];

		if (PORTABILITY_PATH_SEPARATOR(path[i]))
		{
			count = 0;
		}
		else if (path[i] == '.')
		{
			if (i > 0 && path[i - 1] == '.')
			{
				last = 0;
				count = 0;
			}
			else
			{
				if (count > 0)
				{
					last = count - 1;
				}
				else
				{
					last = 0;
				}

				/* This function is the same as portability_path_get_name but
				returns the name of the file without any extension, for example:
					- portability_path_get_name of libnode.so.72 is libnode.so
					- portability_path_get_name_canonical of libnode.so.72 is libnode
				*/
				break;
			}
		}
	}

	if (last == 0 && count > 1)
	{
		last = count;
	}

	name[last] = '\0';

	return last + 1;
}

size_t portability_path_get_fullname(const char *path, size_t path_size, char *name, size_t name_size)
{
	if (path == NULL || name == NULL)
	{
		return 0;
	}

	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' && i < path_size && count < name_size; ++i)
	{
		name[count++] = path[i];

		if (PORTABILITY_PATH_SEPARATOR(path[i]))
		{
			count = 0;
		}
	}

	name[count] = '\0';

	return count + 1;
}

size_t portability_path_get_extension(const char *path, size_t path_size, char *extension, size_t extension_size)
{
	if (path == NULL || extension == NULL)
	{
		return 0;
	}

	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' && i < path_size; ++i)
	{
		extension[count++] = path[i];

		if (PORTABILITY_PATH_SEPARATOR(path[i]) || path[i] == '.' || count == extension_size)
		{
			count = 0;
		}
	}

	extension[count] = '\0';

	return count + 1;
}

size_t portability_path_get_module_name(const char *path, size_t path_size, const char *extension, size_t extension_size, char *name, size_t name_size)
{
	if (path == NULL || extension == NULL || name == NULL)
	{
		return 0;
	}

	if (path_size < extension_size)
	{
		return portability_path_get_fullname(path, path_size, name, name_size);
	}

	size_t i, j;

	for (i = extension_size - 1, j = path_size - 1; i > 0 && j > 0; --i, --j)
	{
		if (extension[i] != path[j])
		{
			return portability_path_get_fullname(path, path_size, name, name_size);
		}
	}

	return portability_path_get_name(path, path_size, name, name_size);
}

size_t portability_path_get_directory(const char *path, size_t path_size, char *absolute, size_t absolute_size)
{
	if (path == NULL || absolute == NULL)
	{
		return 0;
	}

	size_t i, last, size = path_size < absolute_size ? path_size : absolute_size;

	for (i = 0, last = 0; path[i] != '\0' && i < size; ++i)
	{
		absolute[i] = path[i];

		if (PORTABILITY_PATH_SEPARATOR(path[i]))
		{
			last = i + 1;
		}
	}

	absolute[last] = '\0';

	return last + 1;
}

size_t portability_path_get_directory_inplace(char *path, size_t size)
{
	if (path == NULL)
	{
		return 0;
	}

	size_t i, last;

	for (i = 0, last = 0; path[i] != '\0' && i < size; ++i)
	{
		if (PORTABILITY_PATH_SEPARATOR(path[i]))
		{
			last = i + 1;
		}
	}

	path[last] = '\0';

	return last + 1;
}

size_t portability_path_get_relative(const char *base, size_t base_size, const char *path, size_t path_size, char *relative, size_t relative_size)
{
	if (base == NULL || path == NULL || relative == NULL)
	{
		return 0;
	}

	size_t i, length = 0, size = base_size < path_size ? base_size : path_size;

	for (i = 0; base[i] == path[i] && (base[i] != '\0' || path[i] != '\0') && i < size; ++i)
		;

	if (PORTABILITY_PATH_SEPARATOR(path[i]))
	{
		++i;
	}

	for (; path[i] != '\0' && i < relative_size; ++i)
	{
		relative[length++] = path[i];
	}

	relative[length] = '\0';

	return length + 1;
}

int portability_path_is_subpath(const char *parent, size_t parent_size, const char *child, size_t child_size)
{
	if (parent == NULL || child == NULL)
	{
		return 0;
	}

	if (parent_size < child_size)
	{
		return 1;
	}

	return !(strncmp(parent, child, parent_size) == 0);
}

int portability_path_is_absolute(const char *path, size_t size)
{
	if (path == NULL)
	{
		return 1;
	}

#if defined(WIN32) || defined(_WIN32)
	if (size < 3)
	{
		return 1;
	}

	return !((path[0] != '\0' && (path[0] >= 'A' && path[0] <= 'Z')) &&
			 (path[1] != '\0' && path[1] == ':') &&
			 (path[2] != '\0' && PORTABILITY_PATH_SEPARATOR(path[2])));
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) || \
	defined(__HAIKU__) || defined(__BEOS__)

	if (size < 1)
	{
		return 1;
	}

	return !(path[0] != '\0' && PORTABILITY_PATH_SEPARATOR(path[0]));
#else
	#error "Unknown loader path separator"
#endif
}

size_t portability_path_join(const char *left_path, size_t left_path_size, const char *right_path, size_t right_path_size, char *join_path, size_t join_size)
{
	if (left_path == NULL || right_path == NULL || join_path == NULL)
	{
		return 0;
	}

	size_t size = 0;

	if (left_path_size > 1)
	{
		size_t trailing_size = PORTABILITY_PATH_SEPARATOR(left_path[left_path_size - 2]) + PORTABILITY_PATH_SEPARATOR(right_path[0]);

		size += left_path_size - trailing_size;

		if (size > join_size)
		{
			size = join_size;
		}

		memcpy(join_path, left_path, size);

		if (trailing_size == 0)
		{
			join_path[size - 1] = PORTABILITY_PATH_SEPARATOR_C;
		}
	}

	if (right_path_size > 1)
	{
		if (size + right_path_size > join_size)
		{
			right_path_size = join_size - size;
		}

		memcpy(&join_path[size], right_path, right_path_size);

		size += right_path_size;
	}
	else
	{
		if (size + 1 > join_size)
		{
			join_path[size - 1] = '\0';
		}
		else
		{
			join_path[size++] = '\0';
		}
	}

	return size;
}

size_t portability_path_canonical(const char *path, size_t path_size, char *canonical, size_t canonical_size)
{
	if (path == NULL || canonical == NULL)
	{
		return 0;
	}

	size_t iterator, size = 0;
	int separator_found = 1;
	char separator = PORTABILITY_PATH_SEPARATOR_C; /* Use current platform style as default */

	/* Standarize the separators */
	for (iterator = 0; iterator < path_size; ++iterator)
	{
		if (PORTABILITY_PATH_SEPARATOR(path[iterator]))
		{
			separator_found = 0;
			separator = path[iterator];
			break;
		}
	}

	/* Remove first dots */
	for (iterator = 0; path[iterator] == '.' && size < canonical_size; ++iterator)
	{
		size_t next_first = iterator + 1;
		size_t next_second = iterator + 2;

		if (PORTABILITY_PATH_SEPARATOR(path[next_first]))
		{
			++iterator;
			size = 0;
		}
		else if (path[next_first] == '.' && PORTABILITY_PATH_SEPARATOR(path[next_second]))
		{
			iterator += 2;
			size = 0;
		}
		else
		{
			canonical[size++] = path[iterator];
		}
	}

	/* Canonicalize the path */
	for (/* void */; iterator < path_size && size < canonical_size; ++iterator)
	{
		if (path[iterator] == '.')
		{
			size_t next = iterator + 1;

			if (path[next] == '.')
			{
				if (size > 2)
				{
					size -= 2;

					while (size > 0 && !PORTABILITY_PATH_SEPARATOR(canonical[size]))
					{
						--size;
					}

					iterator += 2;
				}
				else
				{
					size = 0;

					if (path[iterator + 3] == '.')
					{
						iterator += 2;
					}
					else
					{
						iterator += 3;
					}
				}
			}
			else if (PORTABILITY_PATH_SEPARATOR(path[next]))
			{
				if (path[next + 1] == '.')
				{
					iterator += 1;

					if (size > 0)
					{
						--size;
					}
				}
				else
				{
					iterator += 2;
				}
			}
			else if (path[next] == '\0')
			{
				if (size > 1)
				{
					--size;
				}

				++iterator;
			}
		}

		/* Store the correct separator */
		canonical[size++] = PORTABILITY_PATH_SEPARATOR(path[iterator]) && separator_found == 0 ? separator : path[iterator];
	}

	return size;
}

int portability_path_separator_normalize_inplace(char *path, size_t size)
{
	if (path == NULL)
	{
		return 1;
	}

	size_t iterator;
	char separator = 0;

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (PORTABILITY_PATH_SEPARATOR_ALL(path[iterator]))
		{
			if (separator == 0)
			{
				separator = PORTABILITY_PATH_SEPARATOR_C; /* Use current platform style as default */
			}
			else
			{
				path[iterator] = separator;
			}
		}
	}

	return 0;
}

int portability_path_compare(const char *left_path, const char *right_path)
{
	if (left_path == NULL || right_path == NULL)
	{
		return 1;
	}

	size_t left_length = strlen(left_path);
	size_t right_length = strlen(right_path);

	if (left_length == 0 || right_length == 0)
	{
		return 1;
	}

	if (PORTABILITY_PATH_SEPARATOR(left_path[left_length - 1]))
	{
		--left_length;
	}

	if (PORTABILITY_PATH_SEPARATOR(right_path[right_length - 1]))
	{
		--right_length;
	}

	if (left_length != right_length)
	{
		return 1;
	}

	return (strncmp(left_path, right_path, left_length) != 0);
}

int portability_path_is_pattern(const char *path, size_t size)
{
	if (path == NULL || size == 0)
	{
		return 1;
	}

	size_t i;

	for (i = 0; path[i] != '\0' && i < size; ++i)
	{
		if (path[i] == '*')
		{
			return 0;
		}
	}

	return 1;
}
