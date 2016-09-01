/*
 *	MetaCall NginX Module Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A NginX module example as metacall HTTP (REST API) wrapper.
 *
 */

/* -- Headers -- */

#include "ngx_http_rest_metacall_module.h"
#include "ngx_http_rest_metacall_uri.h"

#include <string.h>
#include <stdio.h>

#include <metacall/metacall.h>

/* -- Definitions -- */

#define METACALL_NGINX_MSG "MetaCall NginX Module Wrapper\n"

#define METACALL_NGINX_CALL_RESULT_SIZE 0x200

#define METACALL_NGINX_CALL_ARGS_SIZE 0x0A

/* -- Member Data -- */

/**
*  @brief
*    Module command list definition
*/
static ngx_command_t ngx_http_rest_metacall_commands[] =
{
	/* Initialize command */
	{
		ngx_string("metacall_initialize"),	/**< Directive */
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,	/**< Location context and one or more args */
		ngx_http_rest_metacall_initialize_init,	/**< Configuration set up */
		0,					/**< Only one context supported */
		0,					/**< No offset in configuration module struct */
		NULL
	},

	/* Load command */
	{
		ngx_string("metacall_load"),		/**< Directive */
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,	/**< Location context and one or more args */
		ngx_http_rest_metacall_load_init,	/**< Configuration set up */
		0,					/**< Only one context supported */
		0,					/**< No offset in configuration module struct */
		NULL

	},

	/* MetaCall command */
	{
		ngx_string("metacall_call"),		/**< Directive */
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,	/**< Location context and one or more args */
		ngx_http_rest_metacall_call_init,	/**< Configuration set up */
		0,					/**< Only one context supported */
		0,					/**< No offset in configuration module struct */
		NULL
	},

	/* Destroy command */
	{
		ngx_string("metacall_destroy"),		/**< Directive */
		NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,	/**< Location context and one or more args */
		ngx_http_rest_metacall_destroy_init,	/**< Configuration set up */
		0,					/**< Only one context supported */
		0,					/**< No offset in configuration module struct */
		NULL
	},

	ngx_null_command				/**< Command termination */
};

/**
*  @brief
*    Module context definition
*/
static ngx_http_module_t ngx_http_rest_metacall_module_ctx =
{
	NULL,						/**< Pre-configuration */
	NULL,						/**< Post-configuration */

	NULL,						/**< Create main configuration */
	NULL,						/**< Initialize main configuration */

	NULL,						/**< Create server configuration */
	NULL,						/**< Merge server configuration */

	NULL,						/**< Create location configuration */
	NULL						/**< Merge location configuration */
};

/**
*  @brief
*    Module definition
*/
ngx_module_t ngx_http_rest_metacall_module =
{
	NGX_MODULE_V1,					/**< Module version */
	&ngx_http_rest_metacall_module_ctx,		/**< Module context */
	ngx_http_rest_metacall_commands,		/**< Module directives */
	NGX_HTTP_MODULE,				/**< Module type */
	NULL,						/**< Initialize master hook */
	NULL,						/**< Initialize module hook */
	NULL,						/**< Initialize process hook */
	NULL,						/**< Initialize thread hook */
	NULL,						/**< Exit thread hook */
	NULL,						/**< Exit process hook */
	NULL,						/**< Exit master hook */
	NGX_MODULE_V1_PADDING
};

/* -- Methods -- */

static ngx_int_t ngx_http_rest_metacall_initialize_handler(ngx_http_request_t * req)
{
	static const char text_plain[] = "text/plain";
	static const char message_ok[] = METACALL_NGINX_MSG "|>> Correctly initialized\n";
	static const char message_error[] = METACALL_NGINX_MSG "|>> Error in initialization\n";

	ngx_buf_t * buffer;
	ngx_chain_t out;
	u_char * message;
	size_t length;
	ngx_uint_t status;

	/* Set the Content-Type header */
	req->headers_out.content_type.len = sizeof(text_plain) - 1;
	req->headers_out.content_type.data = (u_char *)text_plain;

	/* Allocate new buffer for the response */
	buffer = ngx_pcalloc(req->pool, sizeof(ngx_buf_t));

	/* Insert buffer chain */
	out.buf = buffer;
	out.next = NULL;

	/* Initialize MetaCall */
	if (metacall_initialize() == 0)
	{
		message = (u_char *)message_ok;
		length = sizeof(message_ok);
		status = NGX_HTTP_OK;
	}
	else
	{
		message = (u_char *)message_error;
		length = sizeof(message_error);
		status = NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	/* Set up response content */
	buffer->pos = message;				/* First byte of memory data */
	buffer->last = (u_char *)(message + length);	/* Last byte of memory data */
	buffer->memory = 1;				/* Read-only memory */
	buffer->last_buf = 1;				/* Just one buffer in this request */

	/* Set up response headers */
	req->headers_out.status = status;		/* Result status code */
	req->headers_out.content_length_n = length;	/* Length of the body */

	/* Send headers */
	ngx_http_send_header(req);

	/* Send body and return status code of output filter chain */
	return ngx_http_output_filter(req, &out);
}

static char * ngx_http_rest_metacall_initialize_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	/* Core location configuration */
	ngx_http_core_loc_conf_t * clcf;

	/* Get module location configuration */
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	/* Register module handler */
	clcf->handler = ngx_http_rest_metacall_initialize_handler;

	return NGX_CONF_OK;
}

static ngx_int_t ngx_http_rest_metacall_load_handler(ngx_http_request_t * req)
{
	static const char text_plain[] = "text/plain";
	static const char message[] = METACALL_NGINX_MSG;

	ngx_buf_t * buffer;
	ngx_chain_t out;

	/* Set the Content-Type header */
	req->headers_out.content_type.len = sizeof(text_plain) - 1;
	req->headers_out.content_type.data = (u_char *)text_plain;

	/* Allocate new buffer for the response */
	buffer = ngx_pcalloc(req->pool, sizeof(ngx_buf_t));

	/* Insert buffer chain */
	out.buf = buffer;
	out.next = NULL;

	/* Set up response content */
	buffer->pos = (u_char *)message;			/* First byte of memory data */
	buffer->last = (u_char *)(message + sizeof(message));	/* Last byte of memory data */
	buffer->memory = 1;					/* Read-only memory */
	buffer->last_buf = 1;					/* Just one buffer in this request */

	/* Set up response headers */
	req->headers_out.status = NGX_HTTP_OK;			/* 200 status code */
	req->headers_out.content_length_n = sizeof(message);	/* Length of the body */

	/* Send headers */
	ngx_http_send_header(req);

	/* Send body and return status code of output filter chain */
	return ngx_http_output_filter(req, &out);
}

static char * ngx_http_rest_metacall_load_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	/* Core location configuration */
	ngx_http_core_loc_conf_t * clcf;

	/* Get module location configuration */
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	/* Register module handler */
	clcf->handler = ngx_http_rest_metacall_load_handler;

	return NGX_CONF_OK;
}

static ngx_int_t ngx_http_rest_metacall_call_handler(ngx_http_request_t * req)
{
	static const char text_plain[] = "text/plain";
	static const char message[] = METACALL_NGINX_MSG;

	char data[METACALL_NGINX_CALL_RESULT_SIZE] = { 0 };
	size_t length;

	ngx_buf_t * buffer;
	ngx_chain_t out;

	ngx_uint_t status = NGX_HTTP_OK;

	void * result = NULL;

	u_char ** tokens = NULL;

	size_t size = 0;

	/* Set message data */
	strncat(data, message, sizeof(message));

	/* Execute call from URI request */
	/*if (ngx_http_rest_metacall_uri(req, &result) != 0)*/
	if (ngx_http_rest_metacall_uri_tokenize(req, &tokens, &size) != 0)
	{
		const char invalid_uri_message[] = "|>> Invalid URI\n";

		status = NGX_HTTP_BAD_REQUEST;

		strncat(data, invalid_uri_message, sizeof(invalid_uri_message));
	}
	else
	{
		char text[0xFF];

		const char valid_uri_message[] = "|>> Valid URI\n";

		status = NGX_HTTP_OK;

		strncat(data, valid_uri_message, sizeof(valid_uri_message));

		sprintf(text, "TOKENS: %lu\n", size);

		strncat(data, text, strlen((const char *)text));

		if (tokens != NULL)
		{
			size_t iterator;

			for (iterator = 0; iterator < size; ++iterator)
			{
				strncat(data, (const char *)tokens[iterator], strlen((const char *)tokens[iterator]));

				strncat(data, "\n", 1);
			}

			ngx_http_rest_metacall_uri_tokenize_clear(&tokens, size);
		}
	}

	/* Put result into the message */
	if (result != NULL)
	{
		const char result_message[] = "|>> Result : ";

		/* value v = result; */

		strncat(data, result_message, sizeof(result_message));

		/* Convert value to string */
		/* ... */

		/* Append value string to message */
		/* strncat(data, v_str, v_str_length); */

		strncat(data, "\n", 1);
	}

	/* Retreive data length */
	length = strlen(data);

	/* Set the Content-Type header */
	req->headers_out.content_type.len = sizeof(text_plain) - 1;
	req->headers_out.content_type.data = (u_char *)text_plain;

	/* Allocate new buffer for the response */
	buffer = ngx_pcalloc(req->pool, sizeof(ngx_buf_t));

	/* Insert buffer chain */
	out.buf = buffer;
	out.next = NULL;

	/* Set up response content */
	buffer->pos = (u_char *)data;			/* First byte of memory data */
	buffer->last = (u_char *)(data + length);	/* Last byte of memory data */
	buffer->memory = 1;				/* Read-only memory */
	buffer->last_buf = 1;				/* Just one buffer in this request */

	/* Set up response headers */
	req->headers_out.status = status;		/* Result status code */
	req->headers_out.content_length_n = length;	/* Length of the body */

	/* Send headers */
	ngx_http_send_header(req);

	/* Send body and return status code of output filter chain */
	return ngx_http_output_filter(req, &out);
}

static char * ngx_http_rest_metacall_call_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	/* Core location configuration */
	ngx_http_core_loc_conf_t * clcf;

	/* Get module location configuration */
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	/* Register module handler */
	clcf->handler = ngx_http_rest_metacall_call_handler;

	return NGX_CONF_OK;
}

static ngx_int_t ngx_http_rest_metacall_destroy_handler(ngx_http_request_t * req)
{
	static const char text_plain[] = "text/plain";

	ngx_buf_t * buffer;
	ngx_chain_t out;
	u_char * message;
	size_t length;
	ngx_uint_t status;

	/* Set the Content-Type header */
	req->headers_out.content_type.len = sizeof(text_plain) - 1;
	req->headers_out.content_type.data = (u_char *)text_plain;

	/* Allocate new buffer for the response */
	buffer = ngx_pcalloc(req->pool, sizeof(ngx_buf_t));

	/* Insert buffer chain */
	out.buf = buffer;
	out.next = NULL;

	/* Destroy MetaCall */
	if (metacall_destroy() == 0)
	{
		static const char message_ok[] = METACALL_NGINX_MSG "|>> Correctly destroyed\n";

		message = (u_char *)message_ok;

		length = sizeof(message_ok);

		status = NGX_HTTP_OK;
	}
	else
	{
		static const char message_error[] = METACALL_NGINX_MSG "|>> Error in destruction\n";

		message = (u_char *)message_error;

		length = sizeof(message_error);

		status = NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	/* Set up response content */
	buffer->pos = (u_char *)message;		/* First byte of memory data */
	buffer->last = (u_char *)(message + length);	/* Last byte of memory data */
	buffer->memory = 1;				/* Read-only memory */
	buffer->last_buf = 1;				/* Just one buffer in this request */

	/* Set up response headers */
	req->headers_out.status = status;		/* Result status code */
	req->headers_out.content_length_n = length;	/* Length of the body */

	/* Send headers */
	ngx_http_send_header(req);

	/* Send body and return status code of output filter chain */
	return ngx_http_output_filter(req, &out);
}

static char * ngx_http_rest_metacall_destroy_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	/* Core location configuration */
	ngx_http_core_loc_conf_t * clcf;

	/* Get module location configuration */
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	/* Register module handler */
	clcf->handler = ngx_http_rest_metacall_destroy_handler;

	return NGX_CONF_OK;
}
