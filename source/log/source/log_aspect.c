/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect.h>
#include <log/log_policy.h>
#include <log/log_valid_size.h>

#include <string.h>

/* -- Definitions -- */

#define LOG_ASPECT_POLICIES_MIN_SIZE		((size_t)0x00000001)
#define LOG_ASPECT_POLICIES_DEFAULT_SIZE	((size_t)0x00000010)
#define LOG_ASPECT_POLICIES_MAX_SIZE		((size_t)0x00000100)

/* -- Member Data -- */

struct log_aspect_type
{
	log_aspect_restrict_cb restrict_cb;
	log_policy * policies;
	size_t count;
	size_t size;
	log_aspect_data data;
	log_aspect_interface iface;
};

/* -- Methods -- */

log_aspect log_aspect_create(const log_aspect_interface iface, const log_aspect_ctor ctor)
{
	if (iface != NULL)
	{
		log_aspect aspect = malloc(sizeof(struct log_aspect_type));

		if (aspect == NULL)
		{
			return NULL;
		}

		aspect->restrict_cb = NULL;
		aspect->policies = NULL;
		aspect->count = 0;
		aspect->size = 0;
		aspect->data = NULL;
		aspect->iface = iface;

		if (aspect->iface->create(aspect, ctor) != 0)
		{
			free(aspect);

			return NULL;
		}

		if (aspect->policies == NULL && aspect->size == 0)
		{
			if (log_aspect_reserve(aspect, LOG_ASPECT_POLICIES_DEFAULT_SIZE) != 0)
			{
				free(aspect);

				return NULL;
			}
		}

		return aspect;
	}

	return NULL;
}

int log_aspect_reserve(log_aspect aspect, size_t size)
{
	size = log_valid_size(size);

	if (size < LOG_ASPECT_POLICIES_MIN_SIZE || size > LOG_ASPECT_POLICIES_MAX_SIZE)
	{
		return 1;
	}

	if (aspect->policies != NULL)
	{
		free(aspect->policies);
	}

	aspect->policies = malloc(sizeof(log_policy) * size);

	if (aspect->policies == NULL)
	{
		return 1;
	}

	aspect->count = 0;
	aspect->size = size;

	return 0;
}

void log_aspect_restrict(log_aspect aspect, log_aspect_restrict_cb restrict_cb)
{
	aspect->restrict_cb = restrict_cb;
}

log_aspect_data log_aspect_instance(log_aspect aspect)
{
	return aspect->data;
}

log_aspect_interface log_aspect_behavior(log_aspect aspect)
{
	return aspect->iface;
}

log_aspect_impl log_aspect_derived(log_aspect aspect)
{
	return aspect->iface->impl;
}

int log_aspect_attach(log_aspect aspect, log_policy policy)
{
	if (aspect->restrict_cb != NULL && aspect->restrict_cb(aspect, policy) != 0)
	{
		return 1;
	}

	if ((aspect->count + 1) >= aspect->size)
	{
		register void * data;

		size_t size = 0;

		if (aspect->size >= LOG_ASPECT_POLICIES_MAX_SIZE)
		{
			return 1;
		}

		size = aspect->size << 0x01;

		if ((size - 1) >= LOG_ASPECT_POLICIES_MAX_SIZE)
		{
			size = LOG_ASPECT_POLICIES_MAX_SIZE;
		}

		data = realloc(aspect->policies, size);

		if (data == NULL)
		{
			return 1;
		}

		aspect->policies = data;

		aspect->size = size;
	}

	log_policy_classify(policy, aspect);

	aspect->policies[aspect->count] = policy;

	++aspect->count;

	return 0;
}

int log_aspect_notify_all(log_aspect aspect, log_aspect_notify_cb notify_cb, log_aspect_notify_data notify_data)
{
	int result = 0;

	size_t iterator;

	for (iterator = 0; iterator < aspect->count; ++iterator)
	{
		if (aspect->policies[iterator] != NULL)
		{
			result |= notify_cb(aspect, aspect->policies[iterator], notify_data);
		}
	}

	return result;
}

int log_aspect_detach(log_aspect aspect, log_policy policy)
{
	size_t iterator;

	for (iterator = 0; iterator < aspect->count; ++iterator)
	{
		if (policy == aspect->policies[iterator])
		{
			int result = log_policy_destroy(aspect->policies[iterator]);

			memmove(&aspect->policies[iterator], &aspect->policies[iterator + 1], aspect->count - iterator - 1);

			--aspect->count;

			return result;
		}
	}

	return 1;
}

int log_aspect_destroy(log_aspect aspect)
{
	if (aspect != NULL)
	{
		int result = aspect->iface->destroy(aspect);

		if (aspect->policies)
		{
			size_t iterator;

			for (iterator = 0; iterator < aspect->count; ++iterator)
			{
				result |= log_policy_destroy(aspect->policies[iterator]);
			}

			free(aspect->policies);
		}

		free(aspect);

		return result;
	}

	return 0;
}
