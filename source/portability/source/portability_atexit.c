/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

/* -- Headers -- */

#include <portability/portability_atexit.h>

#include <stdlib.h>

/* -- Member Data -- */

struct atexit_node_type
{
	portability_atexit_fn handler;
	struct atexit_node_type *next;
};

/* -- Private Variables -- */

static struct atexit_node_type *atexit_list = NULL;

/* -- Private Methods -- */

static void portability_atexit_destroy(void)
{
	if (atexit_list != NULL)
	{
		do
		{
			struct atexit_node_type *prev = atexit_list;

			atexit_list = prev->next;

			if (prev->handler != NULL)
			{
				prev->handler();
			}

			free(prev);
		} while (atexit_list != NULL);

		atexit_list = NULL;
	}
}

/* -- Methods -- */

int portability_atexit_initialize(void)
{
	static int atexit_registered = 0;

	if (atexit_list == NULL)
	{
		atexit_list = malloc(sizeof(struct atexit_node_type));

		if (atexit_list == NULL)
		{
			return 1;
		}

		atexit_list->handler = NULL;
		atexit_list->next = NULL;
	}

	if (atexit_registered == 0)
	{
		atexit(&portability_atexit_destroy);
		atexit_registered = 1;
	}

	return 0;
}

int portability_atexit_register(portability_atexit_fn handler)
{
	if (atexit_list == NULL)
	{
		return 1;
	}

	if (atexit_list->handler == NULL)
	{
		atexit_list->handler = handler;
	}
	else
	{
		struct atexit_node_type *iterator = atexit_list;

		/* Find the last or duplicates */
		for (;;)
		{
			if (iterator->handler == handler)
			{
				/* Already registered, skip */
				return 1;
			}

			if (iterator->next == NULL)
			{
				break;
			}

			iterator = iterator->next;
		}

		/* Allocate the new node */
		struct atexit_node_type *node = malloc(sizeof(struct atexit_node_type));

		if (node == NULL)
		{
			return 1;
		}

		node->handler = handler;
		node->next = atexit_list;

		/* Insert it at the begining */
		atexit_list = node;
	}

	return 0;
}
