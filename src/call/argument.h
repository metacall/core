#ifndef ARGUMENT_H
#define ARGUMENT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

enum argument_type_id
{
	TYPE_CHAR	= 0x00,
	TYPE_UCHAR	= 0x01,
	TYPE_SHORT	= 0x02,
	TYPE_USHORT	= 0x03,
	TYPE_INT	= 0x04,
	TYPE_UINT	= 0x05,
	TYPE_LONG	= 0x06,
	TYPE_ULONG	= 0x07,
	TYPE_FLOAT	= 0x08,
	TYPE_DOUBLE	= 0x09,
	TYPE_PTR	= 0x0A,

	TYPE_COUNT
};

typedef void * argument_type_impl;

struct argument_list_type;

typedef struct argument_list_type * argument_list;

typedef argument_type_impl (*argument_type_impl_from_id)(enum argument_type_id);

size_t argument_primitive_size(enum argument_type_id id);

argument_list argument_list_create(argument_type_impl_from_id id_table, int arg_count);

int argument_list_count(argument_list arg_list);

void argument_list_set(argument_list arg_list, int index, enum argument_type_id id, void * data, size_t size);

enum argument_type_id argument_list_get_id(argument_list arg_list, int index);

void * argument_list_get_data(argument_list arg_list, int index);

size_t argument_list_get_size(argument_list arg_list, int index);

argument_type_impl argument_list_get_impl(argument_list arg_list, int index);

void argument_list_destroy(argument_list arg_list);

#ifdef __cplusplus
}
#endif

#endif // ARGUMENT_H
