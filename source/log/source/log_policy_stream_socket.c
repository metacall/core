/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_policy_stream_socket.h>

/* -- Forward Declarations -- */

struct log_policy_stream_socket_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_socket_data_type *log_policy_stream_socket_data;

/* -- Member Data -- */

struct log_policy_stream_socket_data_type
{
	/* TODO */
	void *socket;
};

/* -- Private Methods -- */

static int log_policy_stream_socket_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_socket_write(log_policy policy, const void *buffer, const size_t size);

static int log_policy_stream_socket_flush(log_policy policy);

static int log_policy_stream_socket_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_socket_interface(void)
{
	static struct log_policy_stream_impl_type log_policy_stream_socket_impl_obj = {
		&log_policy_stream_socket_write,
		&log_policy_stream_socket_flush
	};

	static struct log_policy_interface_type policy_interface_stream = {
		&log_policy_stream_socket_create,
		&log_policy_stream_socket_impl_obj,
		&log_policy_stream_socket_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_socket_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_socket_data socket_data = malloc(sizeof(struct log_policy_stream_socket_data_type));

	const log_policy_stream_socket_ctor socket_ctor = ctor;

	if (socket_data == NULL)
	{
		return 1;
	}

/* TODO: open socket */
#define socket_open(ip, port) /* NULL */ (void *)socket_ctor

	socket_data->socket = socket_open(socket_ctor->ip, socket_ctor->port);

	log_policy_instantiate(policy, socket_data, LOG_POLICY_STREAM_SOCKET);

	return 0;
}

static int log_policy_stream_socket_write(log_policy policy, const void *buffer, const size_t size)
{
	log_policy_stream_socket_data socket_data = log_policy_instance(policy);

	/* TODO: write to socket */
	(void)socket_data;
	(void)buffer;
	(void)size;

	return 0;
}

static int log_policy_stream_socket_flush(log_policy policy)
{
	log_policy_stream_socket_data socket_data = log_policy_instance(policy);

	/* TODO: flush socket */
	(void)socket_data;

	return 0;
}

static int log_policy_stream_socket_destroy(log_policy policy)
{
	log_policy_stream_socket_data socket_data = log_policy_instance(policy);

	if (socket_data != NULL)
	{
		/* TODO: close socket */

		free(socket_data);
	}

	return 0;
}
