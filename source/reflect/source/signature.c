/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/signature.h>

#include <string.h>

typedef struct signature_node_type
{
	char * name;
	type t;
} * signature_node;

typedef struct signature_type
{
	size_t count;
} * signature;

signature_node signature_head(signature s)
{
	if (s != NULL)
	{
		return (signature_node)((signature)(s + 1));
	}

	return NULL;
}

signature_node signature_at(signature s, size_t index)
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

	return -1;
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

void signature_set(signature s, size_t index, const char * name, type t)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		if (node->name != NULL)
		{
			free(node->name);
		}

		node->name = strdup(name);

		node->t = t;
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
