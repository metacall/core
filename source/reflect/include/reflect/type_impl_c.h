#ifndef REFLECT_TYPE_INSTANCE_H
#define REFLECT_TYPE_INSTANCE_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <reflect/type_impl.h>

REFLECT_API type_impl type_create_impl_c(type t);

REFLECT_API type_instance type_instance_create_impl_c(type t);

REFLECT_API type_instance type_instance_create_array_impl_c(type t, int count);

REFLECT_API void type_instance_destroy_impl_c(type_instance instance);

REFLECT_API void type_destroy_impl_c(type t, type_impl impl);

REFLECT_API type_interface type_impl_c(void);

#ifdef __cplusplus
}
#endif

#endif // REFLECT_TYPE_INSTANCE_H
