/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_TYPE_H
#define REFLECT_VALUE_TYPE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_value.h>
#include <reflect/reflect_type_id.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Provide type id of value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Return type id assigned to value
*/
REFLECT_API type_id value_type_id(value v);

/**
*  @brief
*    Convert to string the value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[out] dest
*    Destination address where string will be stored
*
*  @param[in] size
*    Size of buffer @dest in bytes
*
*  @param[out] length
*    Length of string stored in @dest
*/
REFLECT_API void value_stringify(value v, char * dest, size_t size, size_t * length);

/**
*  @brief
*    Create a value from boolean @b
*
*  @param[in] b
*    Boolean will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_bool(boolean b);

/**
*  @brief
*    Create a value from char @c
*
*  @param[in] c
*    Character will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_char(char c);

/**
*  @brief
*    Create a value from short @s
*
*  @param[in] s
*    Short will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_short(short s);

/**
*  @brief
*    Create a value from integer @i
*
*  @param[in] i
*    Integer will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_int(int i);

/**
*  @brief
*    Create a value from long @l
*
*  @param[in] l
*    Long integer will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_long(long l);

/**
*  @brief
*    Create a value from double precision floating point number @d
*
*  @param[in] d
*    Double will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_double(double d);

/**
*  @brief
*    Create a value from string @str
*
*  @param[in] str
*    Constant string will be copied into value
*
*  @param[in] length
*    Length of the constant string
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_string(const char * str, size_t length);

/**
*  @brief
*    Create a value from pointer @p
*
*  @param[in] ptr
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_ptr(const void * ptr);

/**
*  @brief
*    Convert value @v to boolean
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to boolean
*/
REFLECT_API boolean value_to_bool(value v);

/**
*  @brief
*    Convert value @v to char
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to char
*/
REFLECT_API char value_to_char(value v);

/**
*  @brief
*    Convert value @v to short
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to short
*/
REFLECT_API short value_to_short(value v);

/**
*  @brief
*    Convert value @v to integer
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to integer
*/
REFLECT_API int value_to_int(value v);

/**
*  @brief
*    Convert value @v to long integer
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to long integer
*/
REFLECT_API long value_to_long(value v);

/**
*  @brief
*    Convert value @v to double precision floating point
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to dobule
*/
REFLECT_API double value_to_double(value v);

/**
*  @brief
*    Convert value @v to string
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to C string
*/
REFLECT_API char * value_to_string(value v);

/**
*  @brief
*    Convert value @v to pointer
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to pointer
*/
REFLECT_API void * value_to_ptr(value v);

/**
*  @brief
*    Assign boolean @b to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] b
*    Boolean to be assigned to value @v
*
*  @return
*    Value with boolean @b assigned to it
*/
REFLECT_API value value_from_bool(value v, boolean b);

/**
*  @brief
*    Assign character @c to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] c
*    Character to be assigned to value @v
*
*  @return
*    Value with char @c assigned to it
*/
REFLECT_API value value_from_char(value v, char c);

/**
*  @brief
*    Assign short @s to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] s
*    Short to be assigned to value @v
*
*  @return
*    Value with short @s assigned to it
*/
REFLECT_API value value_from_short(value v, short s);

/**
*  @brief
*    Assign integer @i to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] i
*    Integer to be assigned to value @v
*
*  @return
*    Value with integer @i assigned to it
*/
REFLECT_API value value_from_int(value v, int i);

/**
*  @brief
*    Assign long integer @l to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] l
*    Long integer to be assigned to value @v
*
*  @return
*    Value with long @l assigned to it
*/
REFLECT_API value value_from_long(value v, long l);

/**
*  @brief
*    Assign double precision floating point @d to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] d
*    Double to be assigned to value @v
*
*  @return
*    Value with double @d assigned to it
*/
REFLECT_API value value_from_double(value v, double d);

/**
*  @brief
*    Assign string @str to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] str
*    Constant string to be assigned to value @v
*
*  @param[in] length
*    Length of the constant string @str
*
*  @return
*    Value with string @str assigned to it
*/
REFLECT_API value value_from_string(value v, const char * str, size_t length);

/**
*  @brief
*    Assign pointer reference @ptr to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] ptr
*    Pointer to be assigned to value @v
*
*  @return
*    Value with pointer @ptr assigned to it
*/
REFLECT_API value value_from_ptr(value v, const void * ptr);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_H */
