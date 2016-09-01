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

int ngx_http_rest_metacall_uri_delimiters(ngx_http_request_t * req, size_t * start, size_t * length);

ngx_str_t * ngx_http_rest_metacall_uri_tokenize_initialize(size_t size);

void ngx_http_rest_metacall_uri_tokenize_clear(ngx_str_t * tokens[], size_t size);

size_t ngx_http_rest_metacall_uri_tokenize_count(ngx_http_request_t * req, size_t start);

int ngx_http_rest_metacall_uri_tokenize_impl(ngx_http_request_t * req, size_t start, ngx_str_t * tokens[], size_t size);

int ngx_http_rest_metacall_uri_tokenize(ngx_http_request_t * req, ngx_str_t * tokens[], size_t * size);

int ngx_http_rest_metacall_uri_parse(ngx_http_request_t * req, char * func_name[], void ** args[]);

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
