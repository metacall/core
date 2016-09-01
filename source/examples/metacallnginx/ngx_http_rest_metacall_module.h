/*
 *	MetaCall NginX Module Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A NginX module example as metacall HTTP (REST API) wrapper.
 *
 */

#ifndef NGX_HTTP_REST_METACALL_MODULE_H
#define NGX_HTTP_REST_METACALL_MODULE_H 1

/* -- Headers -- */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* -- Methods -- */

/**
*  @brief
*    Module metacall initializer handler
*
*  @param[in] req
*    Pointer to request structure
*
*  @return
*    Response generation status
*/
static ngx_int_t ngx_http_rest_metacall_initialize_handler(ngx_http_request_t * req);

/**
*  @brief
*    Module metacall initializer configuration set up
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
static char * ngx_http_rest_metacall_initialize_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

/**
*  @brief
*    Module metacall load handler
*
*  @param[in] req
*    Pointer to request structure
*
*  @return
*    Response generation status
*/
static ngx_int_t ngx_http_rest_metacall_load_handler(ngx_http_request_t * req);

/**
*  @brief
*    Module metacall load configuration set up
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
static char * ngx_http_rest_metacall_load_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

/**
*  @brief
*    Module metacall handler
*
*  @param[in] req
*    Pointer to request structure
*
*  @return
*    Response generation status
*/
static ngx_int_t ngx_http_rest_metacall_call_handler(ngx_http_request_t * req);

/**
*  @brief
*    Module metacall configuration set up
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
static char * ngx_http_rest_metacall_call_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

/**
*  @brief
*    Module metacall destroy handler
*
*  @param[in] req
*    Pointer to request structure
*
*  @return
*    Response generation status
*/
static ngx_int_t ngx_http_rest_metacall_destroy_handler(ngx_http_request_t * req);

/**
*  @brief
*    Module metacall destroy configuration set up
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
static char * ngx_http_rest_metacall_destroy_init(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

#endif /* NGX_HTTP_REST_METACALL_MODULE_H */
