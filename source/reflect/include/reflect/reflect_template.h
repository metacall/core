#ifndef REFLECT_TEMPLATE_H
	#define REFLECT_TEMPLATE_H 1

	#include <reflect/reflect_api.h>
	#include <reflect/reflect_class.h>
	#include <reflect/reflect_function.h>
	#include <reflect/reflect_type.h>

	#include <stddef.h>

	#ifdef __cplusplus
extern "C"
{
	#endif

struct reflect_template_type;

typedef struct reflect_template_type *reflect_template;

typedef enum template_type_id
{
	TEMPLATE_TYPE_FUNCTION,
	TEMPLATE_TYPE_CLASS,
} template_type_id;

typedef enum template_argument_type_id
{
	TEMPLATE_ARGUMENT_TYPE,
	TEMPLATE_ARGUMENT_TEMPLATE
} template_argument_type_id;

typedef struct template_argument_type
{
	const char *name;

	template_argument_type_id kind;

	union
	{
		type value_type;
		reflect_template value_template;
	};
} template_argument;

REFLECT_API reflect_template template_create(
	const char *name,
	template_type_id type);

REFLECT_API const char *template_name(
	reflect_template tpl);

REFLECT_API template_type_id template_type(
	reflect_template tpl);

REFLECT_API void template_destroy(
	reflect_template tpl);

REFLECT_API int template_add_parameter(
	reflect_template tpl,
	const char *parameter);

REFLECT_API size_t template_parameter_count(
	reflect_template tpl);

REFLECT_API const char *template_parameter(
	reflect_template tpl,
	size_t index);

REFLECT_API function template_instantiate_function(
	reflect_template tpl,
	template_argument args[],
	size_t size);

	#ifdef __cplusplus
}
	#endif

#endif