/*
 *	MetaCall NginX Module Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A NginX module example as metacall HTTP (REST API) wrapper.
 *
 */

#ifndef NGX_HTTP_REST_METACALL_URI_H
#define NGX_HTTP_REST_METACALL_URI_H 1

/* -- Headers -- */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* -- Methods -- */

/**
*  @brief
*    Execute a call from URI in request @req
*
*  @param[in] req
*    HTTP request of the call
*
*  @param[out] result
*    Pointer to data from call result
*
*  @return
*    Zero if no error, different from zero otherwhise
*/
int ngx_http_rest_metacall_uri(ngx_http_request_t * req, void ** result);

#endif /* NGX_HTTP_REST_METACALL_URI_H */
