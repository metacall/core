/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_signature.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

typedef struct signature_node_type
{
	char *			name;
	type			t;
} * signature_node;

struct signature_type
{
	type			ret;
	size_t			count;
};

static signature_node signature_head(signature s);

static signature_node signature_at(signature s, size_t index);

static signature_node signature_head(signature s)
{
	if (s != NULL)
	{
		return (signature_node)((signature)(s + 1));
	}

	return NULL;
}

static signature_node signature_at(signature s, size_t index)
{
	signature_node node_list = signature_head(s);

	if (node_list != NULL && index < s->count)
	{
		return &node_list[index];
	}

	return NULL;
}

signature signature_create(size_t count)
{
	signature s = malloc(sizeof(struct signature_type) + sizeof(struct signature_node_type) * count);

	if (s != NULL)
	{
		size_t index;

		s->ret = NULL;

		s->count = count;

		for (index = 0; index < count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL)
			{
				node->name = NULL;
				node->t = NULL;
			}
		}

		return s;
	}

	return NULL;
}

size_t signature_count(signature s)
{
	if (s != NULL)
	{
		return s->count;
	}

	return 0;
}

const char * signature_get_name(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->name;
	}

	return NULL;
}

type signature_get_type(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->t;
	}

	return NULL;
}

type signature_get_return(signature s)
{
	if (s != NULL)
	{
		return s->ret;
	}

	return NULL;
}

void signature_set(signature s, size_t index, const char * name, type t)
{
	if (s != NULL && index < s->count && name != NULL)
	{
		signature_node node = signature_at(s, index);

		size_t name_size = strlen(name) + 1;

		char * name_node = malloc(sizeof(char) * name_size);

		if (name_node == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid name allocation");

			return;
		}

		if (node->name != NULL)
		{
			free(node->name);
		}

		node->name = name_node;

		memcpy(node->name, name, name_size);

		node->t = t;
	}
}

void signature_set_return(signature s, type t)
{
	if (s != NULL)
	{
		s->ret = t;
	}
}

void signature_destroy(signature s)
{
	if (s != NULL)
	{
		size_t index;

		for (index = 0; index < s->count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL && node->name != NULL)
			{
				free(node->name);
			}
		}

		free(s);
	}
}
