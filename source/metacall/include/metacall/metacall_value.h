/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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
	#define boolean unsigned char
#endif

/* -- Enumerations -- */

enum metacall_value_id
{
	METACALL_BOOL = 0,
	METACALL_CHAR = 1,
	METACALL_SHORT = 2,
	METACALL_INT = 3,
	METACALL_LONG = 4,
	METACALL_FLOAT = 5,
	METACALL_DOUBLE = 6,
	METACALL_STRING = 7,
	METACALL_BUFFER = 8,
	METACALL_ARRAY = 9,
	METACALL_MAP = 10,
	METACALL_PTR = 11,
	METACALL_FUTURE = 12,
	METACALL_FUNCTION = 13,
	METACALL_NULL = 14,
	METACALL_CLASS = 15,
	METACALL_OBJECT = 16,
	METACALL_EXCEPTION = 17,
	METACALL_THROWABLE = 18,

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
METACALL_API void *metacall_value_create_bool(boolean b);

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
METACALL_API void *metacall_value_create_char(char c);

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
METACALL_API void *metacall_value_create_short(short s);

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
METACALL_API void *metacall_value_create_int(int i);

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
METACALL_API void *metacall_value_create_long(long l);

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
METACALL_API void *metacall_value_create_float(float f);

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
METACALL_API void *metacall_value_create_double(double d);

/**
*  @brief
*    Create a value from a C string @str
*
*  @param[in] str
*    Constant string will be copied into value (needs to be null terminated)
*
*  @param[in] length
*    Length of the constant string
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_string(const char *str, size_t length);

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
METACALL_API void *metacall_value_create_buffer(const void *buffer, size_t size);

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
METACALL_API void *metacall_value_create_array(const void *values[], size_t size);

/**
*  @brief
*    Create a value map from array of tuples @map
*
*  @param[in] tuples
*    Constant array of tuples will be copied into value map
*
*  @param[in] size
*    Number of elements contained in the map
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_map(const void *tuples[], size_t size);

/**
*  @brief
*    Create a value from pointer @ptr
*
*  @param[in] ptr
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_ptr(const void *ptr);

/**
*  @brief
*    Create a value from future @f
*
*  @param[in] f
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_future(void *f);

/**
*  @brief
*    Create a value from function @f
*
*  @param[in] f
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_function(void *f);

/**
*  @brief
*    Create a value from function @f binding a closure @c to it
*
*  @param[in] f
*    Pointer to constant data will be copied into value
*
*  @param[in] c
*    Pointer to closure that will be binded into function @f
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_function_closure(void *f, void *c);

/**
*  @brief
*    Create a value of type null
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_null(void);

/**
*  @brief
*    Create a value from class @c
*
*  @param[in] c
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_class(void *c);

/**
*  @brief
*    Create a value from object @o
*
*  @param[in] o
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_object(void *o);

/**
*  @brief
*    Create a value from exception @ex
*
*  @param[in] ex
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_exception(void *ex);

/**
*  @brief
*    Create a value from throwable @th
*
*  @param[in] th
*    Pointer to constant data will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
METACALL_API void *metacall_value_create_throwable(void *th);

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
METACALL_API size_t metacall_value_size(void *v);

/**
*  @brief
*    Returns the amount of values this value contains
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Number of values @v represents
*/
METACALL_API size_t metacall_value_count(void *v);

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
METACALL_API enum metacall_value_id metacall_value_id(void *v);

/**
*  @brief
*    Provide type id in a readable form (as string) of a type id
*
*  @param[in] id
*    Value type identifier
*
*  @return
*    Return string related to the type id
*/
METACALL_API const char *metacall_value_id_name(enum metacall_value_id id);

/**
*  @brief
*    Provide type id in a readable form (as string) of value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Return string related to the type id assigned to value
*/
METACALL_API const char *metacall_value_type_name(void *v);

/**
*  @brief
*    Deep copies the value @v, the result copy resets
*    the reference counter and ownership, including the finalizer
*
*  @param[in] v
*    Reference to the value to be copied
*
*  @return
*    Copy of the value @v on success, null otherwhise
*/
METACALL_API void *metacall_value_copy(void *v);

/**
*  @brief
*    Copies the ownership from @src to @dst, including the finalizer,
*    and resets the owner and finalizer of @src
*
*  @param[in] src
*    Source value which will lose the ownership
*
*  @param[in] dst
*    Destination value which will recieve the ownership
*/
METACALL_API void metacall_value_move(void *src, void *dest);

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
METACALL_API boolean metacall_value_to_bool(void *v);

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
METACALL_API char metacall_value_to_char(void *v);

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
METACALL_API short metacall_value_to_short(void *v);

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
METACALL_API int metacall_value_to_int(void *v);

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
METACALL_API long metacall_value_to_long(void *v);

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
METACALL_API float metacall_value_to_float(void *v);

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
METACALL_API double metacall_value_to_double(void *v);

/**
*  @brief
*    Convert value @v to string
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to C string (null terminated)
*/
METACALL_API char *metacall_value_to_string(void *v);

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
METACALL_API void *metacall_value_to_buffer(void *v);

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
METACALL_API void **metacall_value_to_array(void *v);

/**
*  @brief
*    Convert value @v to map
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to map (array of tuples (array of values))
*/
METACALL_API void **metacall_value_to_map(void *v);

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
METACALL_API void *metacall_value_to_ptr(void *v);

/**
*  @brief
*    Convert value @v to future
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to future
*/
METACALL_API void *metacall_value_to_future(void *v);

/**
*  @brief
*    Convert value @v to function
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to function
*/
METACALL_API void *metacall_value_to_function(void *v);

/**
*  @brief
*    Convert value @v to null
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to null
*/
METACALL_API void *metacall_value_to_null(void *v);

/**
*  @brief
*    Convert value @v to class
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to class
*/
METACALL_API void *metacall_value_to_class(void *v);

/**
*  @brief
*    Convert value @v to object
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to object
*/
METACALL_API void *metacall_value_to_object(void *v);

/**
*  @brief
*    Convert value @v to exception
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to exception
*/
METACALL_API void *metacall_value_to_exception(void *v);

/**
*  @brief
*    Convert value @v to throwable
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to throwable
*/
METACALL_API void *metacall_value_to_throwable(void *v);

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
METACALL_API void *metacall_value_from_bool(void *v, boolean b);

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
METACALL_API void *metacall_value_from_char(void *v, char c);

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
METACALL_API void *metacall_value_from_short(void *v, short s);

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
METACALL_API void *metacall_value_from_int(void *v, int i);

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
METACALL_API void *metacall_value_from_long(void *v, long l);

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
METACALL_API void *metacall_value_from_float(void *v, float f);

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
METACALL_API void *metacall_value_from_double(void *v, double d);

/**
*  @brief
*    Assign string @str to value @v, truncates to @v size if it is smaller
*    than @length + 1. It does not add null terminator if truncated.
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] str
*    Constant string to be assigned to value @v (it needs to be null terminated)
*
*  @param[in] length
*    Length of the constant string @str
*
*  @return
*    Value with string @str assigned to it
*/
METACALL_API void *metacall_value_from_string(void *v, const char *str, size_t length);

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
METACALL_API void *metacall_value_from_buffer(void *v, const void *buffer, size_t size);

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
METACALL_API void *metacall_value_from_array(void *v, const void *values[], size_t size);

/**
*  @brief
*    Assign array of values @values to value map @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] tuples
*    Constant array of tuples to be assigned to value map @v
*
*  @param[in] size
*    Number of values contained in constant array @tuples
*
*  @return
*    Value with array of tuples @tuples assigned to it
*/
METACALL_API void *metacall_value_from_map(void *v, const void *tuples[], size_t size);

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
METACALL_API void *metacall_value_from_ptr(void *v, const void *ptr);

/**
*  @brief
*    Assign future @f to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] f
*    Future to be assigned to value @v
*
*  @return
*    Value with future @f assigned to it
*/
METACALL_API void *metacall_value_from_future(void *v, void *f);

/**
*  @brief
*    Assign function @f to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] f
*    Function to be assigned to value @v
*
*  @return
*    Value with function @f assigned to it
*/
METACALL_API void *metacall_value_from_function(void *v, void *f);

/**
*  @brief
*    Assign null to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value with null assigned to it
*/
METACALL_API void *metacall_value_from_null(void *v);

/**
*  @brief
*    Assign class @c to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] c
*    Class to be assigned to value @v
*
*  @return
*    Value with class @c assigned to it
*/
METACALL_API void *metacall_value_from_class(void *v, void *c);

/**
*  @brief
*    Assign object @o to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] o
*    Object to be assigned to value @v
*
*  @return
*    Value with object @o assigned to it
*/
METACALL_API void *metacall_value_from_object(void *v, void *o);

/**
*  @brief
*    Assign exception @ex to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] ex
*    Exception to be assigned to value @v
*
*  @return
*    Value with exception @ex assigned to it
*/
METACALL_API void *metacall_value_from_exception(void *v, void *ex);

/**
*  @brief
*    Assign throwable @th to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] th
*    Throwable to be assigned to value @v
*
*  @return
*    Value with throwable @th assigned to it
*/
METACALL_API void *metacall_value_from_throwable(void *v, void *th);

/**
*  @brief
*    Casts a value to a new type @id
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] id
*    New type id of value to be casted
*
*  @return
*    Casted value or reference to @v if casting is between equivalent types
*/
METACALL_API void *metacall_value_cast(void *v, enum metacall_value_id id);

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
METACALL_API boolean metacall_value_cast_bool(void **v);

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
METACALL_API char metacall_value_cast_char(void **v);

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
METACALL_API short metacall_value_cast_short(void **v);

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
METACALL_API int metacall_value_cast_int(void **v);

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
METACALL_API long metacall_value_cast_long(void **v);

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
METACALL_API float metacall_value_cast_float(void **v);

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
METACALL_API double metacall_value_cast_double(void **v);

/**
*  @brief
*    Convert value @v implicitly to string
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to a C string (null terminated)
*/
METACALL_API char *metacall_value_cast_string(void **v);

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
METACALL_API void *metacall_value_cast_buffer(void **v);

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
METACALL_API void **metacall_value_cast_array(void **v);

/**
*  @brief
*    Convert value @v implicitly to map
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to map
*/
METACALL_API void *metacall_value_cast_map(void **v);

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
METACALL_API void *metacall_value_cast_ptr(void **v);

/**
*  @brief
*    Convert value @v implicitly to future
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to future
*/
METACALL_API void *metacall_value_cast_future(void **v);

/**
*  @brief
*    Convert value @v implicitly to function
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to function
*/
METACALL_API void *metacall_value_cast_function(void **v);

/**
*  @brief
*    Convert value @v implicitly to null
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to null
*/
METACALL_API void *metacall_value_cast_null(void **v);

/**
*  @brief
*    Convert value @v implicitly to class
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to class
*/
METACALL_API void *metacall_value_cast_class(void **v);

/**
*  @brief
*    Convert value @v implicitly to object
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to object
*/
METACALL_API void *metacall_value_cast_object(void **v);

/**
*  @brief
*    Convert value @v implicitly to exception
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to exception
*/
METACALL_API void *metacall_value_cast_exception(void **v);

/**
*  @brief
*    Convert value @v implicitly to throwable
*
*  @param[in] v
*    Reference to the reference of the value
*
*  @return
*    Value converted to throwable
*/
METACALL_API void *metacall_value_cast_throwable(void **v);

/**
*  @brief
*    Destroy a value from scope stack
*
*  @param[in] v
*    Reference to the value
*/
METACALL_API void metacall_value_destroy(void *v);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_VALUE_H */
