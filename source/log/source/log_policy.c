/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <log/log_policy.h>
#include <log/log_aspect.h>

/* -- Member Data -- */

struct log_policy_type
{
	log_aspect aspect;
	enum log_aspect_id aspect_id;
	log_policy_data data;
	log_policy_interface iface;
	log_policy_id id;
};

/* -- Methods -- */

log_policy log_policy_create(enum log_aspect_id aspect_id, const log_policy_interface iface, const log_policy_ctor ctor)
{
	if (iface != NULL)
	{
		log_policy policy = malloc(sizeof(struct log_policy_type));

		if (policy == NULL)
		{
			return NULL;
		}

		policy->aspect = NULL;
		policy->aspect_id = aspect_id;
		policy->data = NULL;
		policy->iface = iface;
		policy->id = 0;

		if (policy->iface->create(policy, ctor) != 0)
		{
			free(policy);

			return NULL;
		}

		return policy;
	}

	return NULL;
}

void log_policy_instantiate(log_policy policy, log_policy_data instance, const log_policy_id id)
{
	policy->data = instance;
	policy->id = id;
}

void log_policy_classify(log_policy policy, log_aspect aspect)
{
	policy->aspect = aspect;
}

log_aspect log_policy_aspect(log_policy policy)
{
	return policy->aspect;
}

enum log_aspect_id log_policy_aspect_id(log_policy policy)
{
	return policy->aspect_id;
}

log_policy_data log_policy_instance(log_policy policy)
{
	return policy->data;
}

log_policy_interface log_policy_behavior(log_policy policy)
{
	return policy->iface;
}

log_policy_impl log_policy_derived(log_policy policy)
{
	return policy->iface->impl;
}

int log_policy_destroy(log_policy policy)
{
	if (policy != NULL)
	{
		int result = policy->iface->destroy(policy);

		free(policy);

		return result;
	}

	return 0;
}
