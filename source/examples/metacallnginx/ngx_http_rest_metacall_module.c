/*
 *	MetaCall NginX Module Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A NginX module example as metacall HTTP (REST API) wrapper.
 *
 */

/* -- Headers -- */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* -- Definitions -- */

#define METACALL_NGINX_MSG "MetaCall NginX Module Wrapper"

/* -- Forward Declarations -- */

static ngx_int_t ngx_http_rest_metacall_handler(ngx_http_request_t * req);

static char * ngx_http_rest_metacall_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

/* -- Member Data -- */

/**
*  @brief
*    Module command list definition
*/
static ngx_command_t ngx_http_rest_metacall_commands[] =
{
	/* Initialize command */
	/*
	{
		ngx_string("metacall_initialize")

	},
	*/

	/* Load command */
	/*
	{
		ngx_string("metacall_load")

	},
	*/

	/* MetaCall command */
	{
		ngx_string("metacall"),			/**< Directive */
		NGX_HTTP_LOC_CONF | NGX_CONF_1MORE,	/**< Location context and one or more args */
		ngx_http_rest_metacall,			/**< Configuration set up */
		0,					/**< Only one context supported */
		0,					/**< No offset in configuration module struct */
		NULL
	},

	/* Destroy command */
	/*
	{
		ngx_string("metacall_destroy")

	},
	*/

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
	&ngx_http_rest_metacall_commands,		/**< Module directives */
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

/**
*  @brief
*    Module handler
*
*  @param[in] req
*    Pointer to request structure
*
*  @return
*    Response generation status
*/
static ngx_int_t ngx_http_rest_metacall_handler(ngx_http_request_t * req)
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
	buffer->pos = message;					/* First byte of memory data */
	buffer->last = mesasge + sizeof(message);		/* Last byte of memory data */
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

/**
*  @brief
*    Module configuration set up
*
*  @param[in] cf
*    Module configuration structure pointer
*
*  @param[in] cmd
*    Module directives structure pointer
*
*  @param[in] conf
*    Module custom configuration memory pointer
*
*  @return
*    Status of configuration set up
*/
static char * ngx_http_rest_metacall_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
	/* Core location configuration */
	ngx_http_core_loc_conf_t * clcf;

	/* Get module location configuration */
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	/* Register module handler */
	clcf->handler = ngx_http_rest_metacall_handler;

	return NGX_CONF_OK;
}
