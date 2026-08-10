#include <reflect/reflect_api.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_class.h>
#include <stddef.h>

#ifndef REFLECT_TEMPLATE_H
#define REFLECT_TEMPLATE_H 1

#ifdef __cplusplus
extern "C" {
    #endif

    typedef enum template_type_id {
        
        TEMPLATE_TYPE_FUNCTION,
        TEMPLATE_TYPE_CLASS,
    } template_type_id;

    typedef struct template_argument_type
    {
        const char *name;
        type value_type;

    } template_argument;

    struct reflect_template_type;

    typedef struct reflect_template_type *reflect_template;

    REFLECT_API reflect_template reflect_template_create(const char *name, template_type_id type);

    REFLECT_API const char *reflect_template_name(reflect_template tpl);

    REFLECT_API template_type_id reflect_template_type(reflect_template tpl);

    REFLECT_API void reflect_template_destroy(reflect_template tpl);

    REFLECT_API int reflect_template_add_parameter(reflect_template tpl, const char *parameter);

    REFLECT_API size_t reflect_template_parameter_count(reflect_template tpl);

    REFLECT_API const char *reflect_template_parameter(reflect_template tpl, size_t index);

    REFLECT_API function reflect_template_instantiate_function(reflect_template tpl, template_argument args[], size_t size);


#ifdef __cplusplus
}
#endif

#endif 
