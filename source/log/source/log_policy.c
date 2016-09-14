/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy.h>
#include <log/log_interface.h>

/* -- Member Data -- */

struct log_policy_type
{
	log_interface iface;
	log_policy_data data;
	log_policy_interface impl;
};

/* -- Methods -- */

log_policy log_policy_create(const log_policy_interface iface, const log_policy_ctor ctor)
{
	if (iface != NULL)
	{
		log_policy policy = malloc(sizeof(log_policy));

		if (policy == NULL)
		{
			return NULL;
		}

		policy->iface = NULL;
		policy->data = NULL;
		policy->impl = iface;

		if (policy->impl->create(policy, ctor) != 0)
		{
			free(policy);

			return NULL;
		}

		return policy;
	}

	return NULL;
}

void log_policy_instantiate(log_policy policy, log_policy_data instance)
{
	policy->data = instance;
}

log_interface log_policy_aspect(log_policy policy)
{
	return policy->iface;
}

log_policy_data log_policy_instance(log_policy policy)
{
	return policy->data;
}

log_policy_impl log_policy_behavior(log_policy policy)
{
	return policy->impl;
}

int log_policy_destroy(log_policy policy)
{
	if (policy != NULL)
	{
		if (policy->impl->destroy(policy) != 0)
		{
			free(policy);

			return 1;
		}

		free(policy);
	}

	return 0;
}
