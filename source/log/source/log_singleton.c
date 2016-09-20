/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <log/log_singleton.h>

/* -- Private Methods -- */

LOG_NO_EXPORT static void log_atexit_callback(void)
{
	int result = log_destroy();

	(void)result;
}

/* -- Protected Methods -- */

static int log_initialize()
{
	log_singleton s = log_instance();

	if (s == NULL)
	{
		abort();
	}

	/* s->log_map = hash_map_create(&hash_callback_str, &comparable_callback_str); */

	return 0;
}

static int log_destroy()
{
	log_singleton s = log_instance();

	if (s == NULL)
	{
		abort();
	}

	/* ... */

	return 0;
}

/* -- Methods -- */

log_singleton log_instance()
{
	log_singleton singleton = NULL;

	#if defined(LOG_THREAD_SAFE)
		/* TODO: Lock */
	#endif

	if (singleton == NULL)
	{
		singleton = malloc(sizeof(struct log_singleton_type));

		if (singleton == NULL)
		{
			log_destroy();
		}

		singleton->map = NULL;
		singleton->size = 0;

		atexit(&log_atexit_callback);

		log_initialize();
	}

	#if defined(LOG_THREAD_SAFE)
		/* TODO: Unlock */
	#endif

	return singleton;
}

