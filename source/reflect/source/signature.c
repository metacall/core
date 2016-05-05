/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#include <reflect/signature.h>

typedef struct signature_node_type
{
	char * name;
	type t;
} * signature_node;

typedef struct signature_type
{
	int count;
} * signature;

signature_node signature_head(signature s)
{
	if (s != NULL)
	{
		return (signature_node)((signature)(s + 1));
	}

	return NULL;
}

signature_node signature_at(signature s, int index)
{
	signature_node node_list = signature_head(s);

	if (node_list != NULL && index >= 0 && index < s->count)
	{
		return &node_list[index];
	}

	return NULL;
}

signature signature_create(int count)
{
	signature s = malloc(sizeof(struct signature_type) + sizeof(struct signature_node_type) * count);

	if (s != NULL)
	{
		int index;

		for (index = 0; index < count; ++index)
		{
			signature_node node = signature_at(s, index);

			node->name = NULL;
			node->t = NULL;
		}

		s->count = count;

		return s;
	}

	return NULL;
}

int signature_count(signature s)
{
	if (s != NULL)
	{
		return s->count;
	}

	return -1;
}

char * signature_get_name(signature s, int index)
{
	if (s != NULL && index >= 0 && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->name;
	}

	return NULL;
}

type signature_get_type(signature s, int index)
{
	if (s != NULL && index >= 0 && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->t;
	}

	return NULL;
}

void signature_set(signature s, int index, char * name, type t)
{
	if (s != NULL && index >= 0 && index < s->count)
	{
		signature_node node = signature_at(s, index);

		node->name = name;
		node->t = t;
	}
}

void signature_destroy(signature s)
{
	if (s != NULL)
	{
		free(s);
	}
}
