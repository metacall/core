/*
 *	MetaCall NginX Module Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A NginX module example as metacall HTTP (REST API) wrapper.
 *
 */

/* -- Headers -- */

#include "ngx_http_rest_metacall_uri.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <metacall/metacall.h>

#include <reflect/value.h>

/* -- Definitions -- */

#define METACALL_NGINX_URI_ARGS_SIZE 0x10
#define METACALL_NGINX_URI_FUNC_SIZE 0xFF
#define METACALL_NGINX_URI_VALUE_STR_SIZE 0xFF

/* -- Private Method Declarations -- */

int ngx_http_rest_metacall_uri_delimiters(ngx_http_request_t * req, size_t * start, size_t * length);

ngx_str_t * ngx_http_rest_metacall_uri_tokenize_initialize(size_t size);

void ngx_http_rest_metacall_uri_tokenize_clear(ngx_str_t * tokens[], size_t size);

size_t ngx_http_rest_metacall_uri_tokenize_count(ngx_http_request_t * req, size_t start);

int ngx_http_rest_metacall_uri_tokenize_impl(ngx_http_request_t * req, size_t start, ngx_str_t * tokens[], size_t size);

int ngx_http_rest_metacall_uri_tokenize(ngx_http_request_t * req, ngx_str_t * tokens[], size_t * size);

void ** ngx_http_rest_metacall_uri_parse_initialize(size_t size);

void ngx_http_rest_metacall_uri_parse_clear(void ** args[], size_t size);

int ngx_http_rest_metacall_uri_parse_token_bool(ngx_str_t * token, int * b);

int ngx_http_rest_metacall_uri_parse_token_int(ngx_str_t * token, int * i);

int ngx_http_rest_metacall_uri_parse_token_str(ngx_str_t * token, char * str);

int ngx_http_rest_metacall_uri_parse_token(ngx_str_t * token, void ** argument);

int ngx_http_rest_metacall_uri_parse_impl(ngx_http_request_t * req, ngx_str_t * tokens[], size_t size, void ** args[]);

int ngx_http_rest_metacall_uri_parse(ngx_http_request_t * req, ngx_str_t * func_name, void ** args[], size_t * size);

/* -- Methods -- */

int ngx_http_rest_metacall_uri_delimiters(ngx_http_request_t * req, size_t * start, size_t * length)
{
	size_t iterator;

	if (start == NULL || length == NULL)
	{
		return 1;
	}

	*start = *length = 0;

	/* Extract position of URI parameters */
	for (iterator = req->uri.len; req->unparsed_uri.data[iterator] != '/' &&
		iterator < req->unparsed_uri.len; ++iterator);

	/* Get URI parameters start */
	if ((iterator + 1) >= req->unparsed_uri.len)
	{
		/* Error */
		*start = req->unparsed_uri.len;

		return 1;
	}
	else
	{
		*start = iterator + 1;
	}

	*length = req->unparsed_uri.len - *start;

	return 0;
}

ngx_str_t * ngx_http_rest_metacall_uri_tokenize_initialize(size_t size)
{
	size_t iterator;

	/* Allocate tokens array */
	ngx_str_t * tokens = malloc(sizeof(ngx_str_t) * size);

	if (tokens == NULL)
	{
		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		tokens[iterator].data = NULL;
		tokens[iterator].len = 0;
	}

	return tokens;
}

void ngx_http_rest_metacall_uri_tokenize_clear(ngx_str_t * tokens[], size_t size)
{
	if (tokens == NULL)
	{
		return;
	}

	size_t iterator;

	/* Clear tokens array */
	for (iterator = 0; iterator < size; ++iterator)
	{
		if ((*tokens)[iterator].data != NULL)
		{
			free((*tokens)[iterator].data);
		}

		(*tokens)[iterator].len = 0;
	}

	free(*tokens);

	*tokens = NULL;
}

size_t ngx_http_rest_metacall_uri_tokenize_count(ngx_http_request_t * req, size_t start)
{
	size_t iterator, size = 0;

	for (iterator = start; iterator <= req->unparsed_uri.len; ++iterator)
	{
		if (req->unparsed_uri.data[iterator] == '/' || iterator == req->unparsed_uri.len)
		{
			++size;
		}
	}

	return size;
}

int ngx_http_rest_metacall_uri_tokenize_impl(ngx_http_request_t * req, size_t start, ngx_str_t * tokens[], size_t size)
{
	size_t iterator, token_count = 0, prev = start;

	for (iterator = start; iterator <= req->unparsed_uri.len; ++iterator)
	{
		if (req->unparsed_uri.data[iterator] == '/' || iterator == req->unparsed_uri.len)
		{
			size_t length = iterator - prev;

			(*tokens)[token_count].data = malloc(sizeof(u_char) * length);

			if ((*tokens)[token_count].data == NULL)
			{
				return 1;
			}

			(*tokens)[token_count].len = length;

			memcpy((*tokens)[token_count].data, &req->unparsed_uri.data[prev], length);

			(*tokens)[token_count].data[length] = '\0';

			prev = iterator + 1;

			++token_count;
		}
	}

	return 0;
}

int ngx_http_rest_metacall_uri_tokenize(ngx_http_request_t * req, ngx_str_t * tokens[], size_t * size)
{
	size_t start, length;

	*size = 0;

	/* Find URI delimiters */
	if (ngx_http_rest_metacall_uri_delimiters(req, &start, &length) != 0)
	{
		/* Error */
		return 1;
	}

	/* Find number of tokens in URI */
	if (length > 0)
	{
		size_t token_count = ngx_http_rest_metacall_uri_tokenize_count(req, start);

		if (token_count == 0)
		{
			return 1;
		}

		*size = token_count;
	}
	else
	{
		return 1;
	}

	/* Allocate tokens array */
	*tokens = ngx_http_rest_metacall_uri_tokenize_initialize(*size);

	if (*tokens == NULL)
	{
		return 1;
	}

	/* Split URI in tokens */
	if (ngx_http_rest_metacall_uri_tokenize_impl(req, start, tokens, *size) != 0)
	{
		/* Error */
		ngx_http_rest_metacall_uri_tokenize_clear(tokens, *size);

		return 1;
	}

	return 0;
}

void ** ngx_http_rest_metacall_uri_parse_initialize(size_t size)
{
	size_t iterator;

	void ** args = malloc(sizeof(void *) * size);

	if (args == NULL)
	{
		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		args[iterator] = NULL;
	}

	return args;
}

void ngx_http_rest_metacall_uri_parse_clear(void ** args[], size_t size)
{
	if (args == NULL)
	{
		return;
	}

	if (*args != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < size; ++iterator)
		{
			if ((*args)[iterator] != NULL)
			{
				value_destroy((value)((*args)[iterator]));
			}
		}

		free(*args);

		*args = NULL;
	}
}

int ngx_http_rest_metacall_uri_parse_token_bool(ngx_str_t * token, int * b)
{
	static const char true_str[] = "true";

	static const char false_str[] = "false";

	size_t length = token->len < sizeof(true_str) ? token->len : sizeof(true_str);

	if (strncmp((const char *)token->data, true_str, length) == 0)
	{
		*b = 1;

		return 0;
	}

	if (strncmp((const char *)token->data, false_str, length) == 0)
	{
		*b = 0;

		return 0;
	}

	*b = -1;

	return 1;
}

int ngx_http_rest_metacall_uri_parse_token_int(ngx_str_t * token, int * i)
{
	size_t iterator;

	for (iterator = 0; iterator < token->len; ++iterator)
	{
		if (isdigit((int)token->data[iterator]) == 0)
		{
			*i = -1;

			return 1;
		}
	}

	*i = atoi((const char *)token->data);

	return 0;
}

int ngx_http_rest_metacall_uri_parse_token_double(ngx_str_t * token, double * d)
{
	size_t iterator;

	for (iterator = 0; iterator < token->len; ++iterator)
	{
		if (isdigit((int)token->data[iterator]) == 0 &&
			token->data[iterator] != '.' &&
			token->data[iterator] != '-' &&
			token->data[iterator] != '+' &&
			token->data[iterator] != 'E')
		{
			*d = 0.0;

			return 1;
		}
	}

	*d = atof((const char *)token->data);

	return 0;
}

int ngx_http_rest_metacall_uri_parse_token_str(ngx_str_t * token, char * str)
{
	size_t iterator;

	for (iterator = 0; iterator < token->len; ++iterator)
	{
		if (isalnum((int)token->data[iterator]) == 0)
		{
			*str = '\0';

			return 1;
		}
	}

	memcpy(str, (const char *)token->data, token->len);

	return 0;
}

int ngx_http_rest_metacall_uri_parse_token(ngx_str_t * token, void ** argument)
{
	int int_value = 0;

	double double_value = 0.0;

	char str_value[METACALL_NGINX_URI_VALUE_STR_SIZE] = { 0 };

	if (ngx_http_rest_metacall_uri_parse_token_bool(token, &int_value) == 0)
	{
		*argument = value_create_bool((boolean)int_value);

		return 0;
	}

	if (ngx_http_rest_metacall_uri_parse_token_int(token, &int_value) == 0)
	{
		*argument = value_create_int(int_value);

		return 0;
	}

	if (ngx_http_rest_metacall_uri_parse_token_double(token, &double_value) == 0)
	{
		*argument = value_create_double(double_value);

		return 0;
	}

	if (ngx_http_rest_metacall_uri_parse_token_str(token, str_value) == 0)
	{
		size_t str_length = strnlen((const char *)str_value, METACALL_NGINX_URI_VALUE_STR_SIZE);

		*argument = value_create_string(str_value, str_length);

		return 0;
	}

	/* Invalid token */
	*argument = NULL;

	return 1;
}

int ngx_http_rest_metacall_uri_parse_impl(ngx_http_request_t * req, ngx_str_t * tokens[], size_t size, void ** args[])
{
	size_t iterator;

	/* First token is the method name, so begin by first parameter */
	for (iterator = 1; iterator < size; ++iterator)
	{
		/* Parse each token and generate arguments */
		if (ngx_http_rest_metacall_uri_parse_token(&((*tokens)[iterator]), &((*args)[iterator])) != 0)
		{
			/* Clear arguments */
			ngx_http_rest_metacall_uri_parse_clear(args, size);

			return 1;
		}
	}

	return 0;
}

int ngx_http_rest_metacall_uri_parse(ngx_http_request_t * req, ngx_str_t * func_name, void ** args[], size_t * size)
{
	ngx_str_t * tokens = NULL;

	*size = 0;

	/* Tokenize URI */
	if (ngx_http_rest_metacall_uri_tokenize(req, (ngx_str_t **)&tokens, size) == 0)
	{
		/* Copy function name */
		func_name->len = tokens[0].len;

		func_name->data = malloc(sizeof(u_char) * (func_name->len + 1));

		if (func_name->data == NULL)
		{
			/* Clear tokens */
			ngx_http_rest_metacall_uri_tokenize_clear((ngx_str_t **)&tokens, *size);

			return 1;
		}

		memcpy(func_name->data, tokens[0].data, func_name->len);

		func_name->data[func_name->len] = '\0';

		/* Initialize arguments */
		*args = ngx_http_rest_metacall_uri_parse_initialize(*size);

		if (*args == NULL)
		{
			/* Error */
			ngx_http_rest_metacall_uri_tokenize_clear((ngx_str_t **)&tokens, *size);

			free(func_name->data);

			return 1;
		}

		/* Parse tokens into arguments */
		if (ngx_http_rest_metacall_uri_parse_impl(req, (ngx_str_t **)&tokens, *size, args) != 0)
		{
			/* Error */
			ngx_http_rest_metacall_uri_tokenize_clear((ngx_str_t **)&tokens, *size);

			free(func_name->data);

			return 1;
		}

		/* Clear tokens */
		ngx_http_rest_metacall_uri_tokenize_clear((ngx_str_t **)&tokens, *size);

		return 0;
	}

	return 1;
}

int ngx_http_rest_metacall_uri(ngx_http_request_t * req, void ** result)
{
	ngx_str_t func_name;

	void ** args = NULL;

	size_t size = 0;

	/* Parse URI into function name and array of arguments */
	if (ngx_http_rest_metacall_uri_parse(req, &func_name, &args, &size) != 0)
	{
		/* Error */
		*result = NULL;

		return 1;
	}

	/* Execute the call */
	*result = metacallv((const char *)func_name.data, args);

	/* Clear arguments */
	ngx_http_rest_metacall_uri_parse_clear(&args, size);

	/* Clear function name */
	free(func_name.data);

	return 0;
}
