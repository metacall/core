/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_future.h>

#include <threading/threading_atomic_ref_count.h>

#include <reflect/reflect_memory_tracker.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct future_type
{
	future_impl impl;
	future_interface interface;
	threading_atomic_ref_count_type ref;
};

reflect_memory_tracker(future_stats);

future future_create(future_impl impl, future_impl_interface_singleton singleton)
{
	future f = malloc(sizeof(struct future_type));

	if (f == NULL)
	{
		return NULL;
	}

	f->impl = impl;
	threading_atomic_ref_count_initialize(&f->ref);

	if (singleton)
	{
		f->interface = singleton();
	}
	else
	{
		f->interface = NULL;
	}

	if (f->interface != NULL && f->interface->create != NULL)
	{
		if (f->interface->create(f, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid future create callback <%p>", f->interface->create);

			threading_atomic_ref_count_destroy(&f->ref);

			free(f);

			return NULL;
		}
	}

	reflect_memory_tracker_allocation(future_stats);

	return f;
}

int future_increment_reference(future f)
{
	if (f == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_increment(&f->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_increment(future_stats);

	return 0;
}

int future_decrement_reference(future f)
{
	if (f == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_decrement(&f->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_decrement(future_stats);

	return 0;
}

future_return future_await(future f, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void *context)
{
	if (f != NULL)
	{
		if (f->interface != NULL && f->interface->await != NULL)
		{
			/* TODO: Disable logs here until log is completely thread safe and async signal safe */

			/*
			log_write("metacall", LOG_LEVEL_DEBUG, "Await future (%p)", (void *)f);
			*/

			return f->interface->await(f, f->impl, resolve_callback, reject_callback, context);
		}
	}

	return NULL;
}

void future_stats_debug(void)
{
	reflect_memory_tracker_print(future_stats, "FUTURES");
}

void future_destroy(future f)
{
	if (f != NULL)
	{
		if (future_decrement_reference(f) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in future <%p>", (void *)f);
		}

		if (threading_atomic_ref_count_load(&f->ref) == 0)
		{
			if (f->interface != NULL && f->interface->destroy != NULL)
			{
				f->interface->destroy(f, f->impl);
			}

			threading_atomic_ref_count_destroy(&f->ref);

			free(f);

			reflect_memory_tracker_deallocation(future_stats);
		}
	}
}
