/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_path.h>

#include <string.h>

#if defined(WIN32) || defined(_WIN32) ||            \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define LOADER_PATH_SEPARATOR(chr) (chr == '\\' || chr == '/')
	#define LOADER_PATH_SEPARATOR_C	   '/'

#elif defined(unix) || defined(__unix__) || defined(__unix) ||                          \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) ||                                     \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) ||                 \
	defined(__HAIKU__) || defined(__BEOS__)
	#define LOADER_PATH_SEPARATOR(chr) (chr == '/')
	#define LOADER_PATH_SEPARATOR_C	   '/'

#else
	#error "Unknown path separator"
#endif

size_t loader_path_get_name(const loader_naming_path path, loader_naming_name name)
{
	size_t i, count, last;

	for (i = 0, count = 0, last = 0; path[i] != '\0' &&
									 i < LOADER_NAMING_PATH_SIZE && count < LOADER_NAMING_NAME_SIZE;
		 ++i)
	{
		name[count++] = path[i];

		if (LOADER_PATH_SEPARATOR(path[i]))
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

size_t loader_path_get_fullname(const loader_naming_path path, loader_naming_name name)
{
	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' &&
						   i < LOADER_NAMING_PATH_SIZE && count < LOADER_NAMING_NAME_SIZE;
		 ++i)
	{
		name[count++] = path[i];

		if (LOADER_PATH_SEPARATOR(path[i]))
		{
			count = 0;
		}
	}

	name[count] = '\0';

	return count + 1;
}

size_t loader_path_get_extension(const loader_naming_path path, loader_naming_tag extension)
{
	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' &&
						   i < LOADER_NAMING_PATH_SIZE;
		 ++i)
	{
		extension[count++] = path[i];

		if (LOADER_PATH_SEPARATOR(path[i]) || path[i] == '.' || count == LOADER_NAMING_TAG_SIZE)
		{
			count = 0;
		}
	}

	extension[count] = '\0';

	return count + 1;
}

size_t loader_path_get_module_name(const loader_naming_path path, loader_naming_name name, const loader_naming_tag extension)
{
	loader_naming_tag name_extension;

	size_t i, size = loader_path_get_extension(path, name_extension);

	for (i = 0; i < size && extension[i] != '\0'; ++i)
	{
		if (name_extension[i] != extension[i])
		{
			return loader_path_get_fullname(path, name);
		}
	}

	return loader_path_get_name(path, name);
}

size_t loader_path_get_path(const loader_naming_path path, size_t size, loader_naming_path absolute)
{
	size_t i, last, path_size = size > LOADER_NAMING_PATH_SIZE ? LOADER_NAMING_PATH_SIZE : size;

	for (i = 0, last = 0; path[i] != '\0' && i < path_size; ++i)
	{
		absolute[i] = path[i];

		if (LOADER_PATH_SEPARATOR(path[i]))
		{
			last = i + 1;
		}
	}

	absolute[last] = '\0';

	return last + 1;
}

size_t loader_path_get_relative(const loader_naming_path base, const loader_naming_path path, loader_naming_path relative)
{
	size_t i, length = 0;

	for (i = 0; base[i] == path[i] && (base[i] != '\0' || path[i] != '\0') && i < LOADER_NAMING_PATH_SIZE; ++i)
		;

	if (LOADER_PATH_SEPARATOR(path[i]))
	{
		++i;
	}

	for (; path[i] != '\0' && i < LOADER_NAMING_PATH_SIZE; ++i)
	{
		relative[length++] = path[i];
	}

	relative[length] = '\0';

	return length + 1;
}

int loader_path_is_subpath(const loader_naming_path parent, const loader_naming_path child)
{
	size_t parent_size = strnlen(parent, LOADER_NAMING_PATH_SIZE);
	size_t child_size = strnlen(child, LOADER_NAMING_PATH_SIZE);

	if (parent_size < child_size)
	{
		return 1;
	}

	return !(strncmp(parent, child, parent_size) == 0);
}

int loader_path_is_absolute(const loader_naming_path path)
{
#if defined(WIN32) || defined(_WIN32)
	return !((path[0] != '\0' && (path[0] >= 'A' && path[0] <= 'Z')) &&
			 (path[1] != '\0' && path[1] == ':') &&
			 (path[2] != '\0' && LOADER_PATH_SEPARATOR(path[2])));
#elif defined(unix) || defined(__unix__) || defined(__unix) ||                          \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) ||                                     \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) ||                 \
	defined(__HAIKU__) || defined(__BEOS__)
	return !(path[0] != '\0' && LOADER_PATH_SEPARATOR(path[0]));
#else
	#error "Unknown loader path separator"
#endif
}

size_t loader_path_join(const loader_naming_path left_path, size_t left_path_size, const loader_naming_path right_path, size_t right_path_size, loader_naming_path join_path)
{
	size_t size = 0;

	if (left_path_size > 1)
	{
		size_t trailing_size = LOADER_PATH_SEPARATOR(left_path[left_path_size - 2]) + LOADER_PATH_SEPARATOR(right_path[0]);

		size += left_path_size - trailing_size;

		memcpy(join_path, left_path, size);

		if (trailing_size == 0)
		{
			join_path[size - 1] = LOADER_PATH_SEPARATOR_C;
		}
	}

	if (right_path_size > 1)
	{
		memcpy(&join_path[size], right_path, right_path_size);

		size += right_path_size;
	}
	else
	{
		join_path[size++] = '\0';
	}

	return size;
}

size_t loader_path_canonical(const loader_naming_path path, size_t size, loader_naming_path canonical)
{
	size_t iterator, canonical_size = 0;
	int separator_found = 1;
	char separator = LOADER_PATH_SEPARATOR_C; /* Use current platform style as default */

	/* Standarize the separators */
	for (iterator = 0; iterator < size; ++iterator)
	{
		if (LOADER_PATH_SEPARATOR(path[iterator]))
		{
			separator_found = 0;
			separator = path[iterator];
			break;
		}
	}

	/* Remove first dots */
	for (iterator = 0; path[iterator] == '.'; ++iterator)
	{
		size_t next_first = iterator + 1;
		size_t next_second = iterator + 2;

		if (LOADER_PATH_SEPARATOR(path[next_first]))
		{
			++iterator;
			canonical_size = 0;
		}
		else if (path[next_first] == '.' && LOADER_PATH_SEPARATOR(path[next_second]))
		{
			iterator += 2;
			canonical_size = 0;
		}
		else
		{
			canonical[canonical_size++] = path[iterator];
		}
	}

	/* Canonicalize the path */
	for (/* void */; iterator < size; ++iterator)
	{
		if (path[iterator] == '.')
		{
			size_t next = iterator + 1;

			if (path[next] == '.')
			{
				if (canonical_size > 2)
				{
					canonical_size -= 2;

					while (canonical_size > 0 && !LOADER_PATH_SEPARATOR(canonical[canonical_size]))
					{
						--canonical_size;
					}

					iterator += 2;
				}
				else
				{
					canonical_size = 0;

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
			else if (LOADER_PATH_SEPARATOR(path[next]))
			{
				if (path[next + 1] == '.')
				{
					iterator += 1;

					if (canonical_size > 0)
					{
						--canonical_size;
					}
				}
				else
				{
					iterator += 2;
				}
			}
			else if (path[next] == '\0')
			{
				if (canonical_size > 1)
				{
					--canonical_size;
				}

				++iterator;
			}
		}

		/* Store the correct separator */
		canonical[canonical_size++] = LOADER_PATH_SEPARATOR(path[iterator]) && separator_found == 0 ? separator : path[iterator];
	}

	return canonical_size;
}
