/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#ifndef METACALL_VALUE_H
#define METACALL_VALUE_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */

#ifndef boolean
#	define boolean unsigned char
#endif

/* -- Enumerations -- */

enum metacall_value_id
{
	METACALL_BOOL	= 0,
	METACALL_CHAR	= 1,
	METACALL_SHORT	= 2,
	METACALL_INT	= 3,
	METACALL_LONG	= 4,
	METACALL_FLOAT	= 5,
	METACALL_DOUBLE	= 6,
	METACALL_STRING	= 7,
	METACALL_BUFFER	= 8,
	METACALL_ARRAY	= 9,
	METACALL_PTR	= 10,

	METACALL_SIZE,
	METACALL_INVALID
};

/* -- Methods -- */

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
METACALL_API void * metacall_value_create_bool(boolean b);

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
METACALL_API void * metacall_value_create_char(char c);

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
METACALL_API void * metacall_value_create_short(short s);

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
METACALL_API void * metacall_value_create_int(int i);

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
METACALL_API void * metacall_value_create_long(long l);

/**
*  @brief
*    Create a value from single precision floating point number @f
*
*  @param[in] f
*    Float will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void * metacall_value_create_float(float f);

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
METACALL_API void * metacall_value_create_double(double d);

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
METACALL_API void * metacall_value_create_string(const char * str, size_t length);

/**
*  @brief
*    Create a value buffer from array @buffer
*
*  @param[in] buffer
*    Constant memory block will be copied into value array
*
*  @param[in] size
*    Size in bytes of data contained in the array
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void * metacall_value_create_buffer(const void * buffer, size_t size);

/**
*  @brief
*    Create a value array from array of values @values
*
*  @param[in] values
*    Constant array of values will be copied into value list
*
*  @param[in] size
*    Number of elements contained in the array
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void * metacall_value_create_array(const void * values[], size_t size);

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
METACALL_API void * metacall_value_create_ptr(const void * ptr);

/**
*  @brief
*    Returns the size of the value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Size in bytes of the value
*/
METACALL_API size_t metacall_value_size(void * v);

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
METACALL_API enum metacall_value_id metacall_value_id(void * v);

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
METACALL_API boolean metacall_value_to_bool(void * v);

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
METACALL_API char metacall_value_to_char(void * v);

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
METACALL_API short metacall_value_to_short(void * v);

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
METACALL_API int metacall_value_to_int(void * v);

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
METACALL_API long metacall_value_to_long(void * v);

/**
*  @brief
*    Convert value @v to single precision floating point
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to float
*/
METACALL_API float metacall_value_to_float(void * v);

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
METACALL_API double metacall_value_to_double(void * v);

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
METACALL_API char * metacall_value_to_string(void * v);

/**
*  @brief
*    Convert value @v to buffer
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to memory block
*/
METACALL_API void * metacall_value_to_buffer(void * v);

/**
*  @brief
*    Convert value @v to array of values
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to array of values
*/
METACALL_API void ** metacall_value_to_array(void * v);

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
METACALL_API void * metacall_value_to_ptr(void * v);

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
METACALL_API void * metacall_value_from_bool(void * v, boolean b);

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
METACALL_API void * metacall_value_from_char(void * v, char c);

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
METACALL_API void * metacall_value_from_short(void * v, short s);

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
METACALL_API void * metacall_value_from_int(void * v, int i);

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
METACALL_API void * metacall_value_from_long(void * v, long l);

/**
*  @brief
*    Assign single precision floating point @f to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] f
*    Float to be assigned to value @v
*
*  @return
*    Value with float @f assigned to it
*/
METACALL_API void * metacall_value_from_float(void * v, float f);

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
METACALL_API void * metacall_value_from_double(void * v, double d);

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
METACALL_API void * metacall_value_from_string(void * v, const char * str, size_t length);

/**
*  @brief
*    Assign array @buffer to value buffer @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] buffer
*    Constant array to be assigned to value @v
*
*  @param[in] size
*    Number of elements contained in @buffer
*
*  @return
*    Value with array @buffer assigned to it
*/
METACALL_API void * metacall_value_from_buffer(void * v, const void * buffer, size_t size);

/**
*  @brief
*    Assign array of values @values to value array @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] values
*    Constant array of values to be assigned to value array @v
*
*  @param[in] size
*    Number of values contained in constant array @values
*
*  @return
*    Value with array of values @values assigned to it
*/
METACALL_API void * metacall_value_from_array(void * v, const void * values[], size_t size);

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
METACALL_API void * metacall_value_from_ptr(void * v, const void * ptr);

/**
*  @brief
*    Convert value @v implicitly to boolean
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to boolean
*/
METACALL_API boolean metacall_value_cast_bool(void ** v);

/**
*  @brief
*    Convert value @v implicitly to char
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to char
*/
METACALL_API char metacall_value_cast_char(void ** v);

/**
*  @brief
*    Convert value @v implicitly to short
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to short
*/
METACALL_API short metacall_value_cast_short(void ** v);

/**
*  @brief
*    Convert value @v implicitly to int
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to int
*/
METACALL_API int metacall_value_cast_int(void ** v);

/**
*  @brief
*    Convert value @v implicitly to long
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to long
*/
METACALL_API long metacall_value_cast_long(void ** v);

/**
*  @brief
*    Convert value @v implicitly to float
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to float
*/
METACALL_API float metacall_value_cast_float(void ** v);

/**
*  @brief
*    Convert value @v implicitly to double
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to double
*/
METACALL_API double metacall_value_cast_double(void ** v);

/**
*  @brief
*    Convert value @v implicitly to string
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to string
*/
METACALL_API char * metacall_value_cast_string(void ** v);

/**
*  @brief
*    Convert value @v implicitly to buffer
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to buffer
*/
METACALL_API void * metacall_value_cast_buffer(void ** v);

/**
*  @brief
*    Convert value @v implicitly to array
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to array of values
*/
METACALL_API void ** metacall_value_cast_array(void ** v);

/**
*  @brief
*    Convert value @v implicitly to ptr
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to ptr
*/
METACALL_API void * metacall_value_cast_ptr(void ** v);

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
METACALL_API void metacall_value_stringify(void * v, char * dest, size_t size, size_t * length);

/**
*  @brief
*    Destroy a value from scope stack
*
*  @param[in] v
*    Reference to the value
*/
METACALL_API void metacall_value_destroy(void * v);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_VALUE_H */
