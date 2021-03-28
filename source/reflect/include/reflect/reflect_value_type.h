/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_VALUE_TYPE_H
#define REFLECT_VALUE_TYPE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_future.h>
#include <reflect/reflect_object.h>
#include <reflect/reflect_type_id.h>
#include <reflect/reflect_value.h>
#include <reflect/reflect_value_type_cast.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Create a value type from @data with size @bytes and typeid @id
*
*  @param[in] data
*    Pointer to memory block
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*
*  @param[in] id
*    Type of memory block @data
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_type_create(const void *data, size_t bytes, type_id id);

/**
*  @brief
*    Make a deep copy of value @v
*
*  @param[in] v
*    Reference to the value is going to be copied
*
*  @return
*    Pointer to a deep copy of new value if success, null otherwhise
*/
REFLECT_API value value_type_copy(value v);

/**
*  @brief
*    Returns the size of the value type
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Size in bytes of the value type
*/
REFLECT_API size_t value_type_size(value v);

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
REFLECT_API size_t value_type_count(void *v);

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
*    Create a value from single precision floating point number @f
*
*  @param[in] f
*    Float will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_float(float f);

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
REFLECT_API value value_create_string(const char *str, size_t length);

/**
*  @brief
*    Create a value buffer from array @buffer
*
*  @param[in] arr
*    Constant memory block will be copied into value buffer
*
*  @param[in] size
*    Size in bytes of data contained in the array
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_buffer(const void *buffer, size_t size);

/**
*  @brief
*    Create a value array from array of values @values
*
*  @param[in] values
*    Constant array of values will be copied into value array
*
*  @param[in] size
*    Number of elements contained in the array
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_array(const value *values, size_t size);

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
REFLECT_API value value_create_map(const value *tuples, size_t size);

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
REFLECT_API value value_create_ptr(const void *ptr);

/**
*  @brief
*    Create a value from future @f
*
*  @param[in] f
*    Pointer to future will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_future(future f);

/**
*  @brief
*    Create a value from function @f
*
*  @param[in] f
*    Pointer to function will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_function(function f);

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
REFLECT_API value value_create_function_closure(function f, void *c);

/**
*  @brief
*    Create a value of type null
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_null(void);

/**
*  @brief
*    Create a value from function @c
*
*  @param[in] c
*    Pointer to class will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_class(klass c);

/**
*  @brief
*    Create a value from object @o
*
*  @param[in] o
*    Pointer to object will be copied into value
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create_object(object o);

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
*    Convert value @v to single precision floating point
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Value converted to float
*/
REFLECT_API float value_to_float(value v);

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
*    Value converted to a C string (null terminated)
*/
REFLECT_API char *value_to_string(value v);

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
REFLECT_API void *value_to_buffer(value v);

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
REFLECT_API value *value_to_array(value v);

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
REFLECT_API value *value_to_map(value v);

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
REFLECT_API void *value_to_ptr(value v);

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
REFLECT_API future value_to_future(value v);

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
REFLECT_API function value_to_function(value v);

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
REFLECT_API void *value_to_null(value v);

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
REFLECT_API klass value_to_class(value v);

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
REFLECT_API object value_to_object(value v);

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
REFLECT_API value value_from_float(value v, float f);

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
REFLECT_API value value_from_string(value v, const char *str, size_t length);

/**
*  @brief
*    Assign array @buffer to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] buffer
*    Constant byte array to be assigned to value @v
*
*  @param[in] size
*    Size in bytes of data contained in @buffer
*
*  @return
*    Value with array @buffer assigned to it
*/
REFLECT_API value value_from_buffer(value v, const void *buffer, size_t size);

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
REFLECT_API value value_from_array(value v, const value *values, size_t size);

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
REFLECT_API value value_from_map(value v, const value *tuples, size_t size);

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
REFLECT_API value value_from_ptr(value v, const void *ptr);

/**
*  @brief
*    Assign future reference @f to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] f
*    Future to be assigned to value @v
*
*  @return
*    Value with future @future assigned to it
*/
REFLECT_API value value_from_future(value v, future f);

/**
*  @brief
*    Assign function reference @f to value @v
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
REFLECT_API value value_from_function(value v, function f);

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
REFLECT_API value value_from_null(value v);

/**
*  @brief
*    Assign class reference @c to value @v
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
REFLECT_API value value_from_class(value v, klass c);

/**
*  @brief
*    Assign object reference @o to value @v
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
REFLECT_API value value_from_object(value v, object o);

/**
*  @brief
*    Destroy recursively a value type @v
*
*  @param[in] v
*    Reference to the value
*
*/
REFLECT_API void value_type_destroy(value v);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_H */
