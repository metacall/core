/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_H
#define REFLECT_VALUE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Type Definitions -- */

typedef int boolean;

typedef void * value;

/* -- Methods -- */

/**
*  @brief
*    Create a value from @data with size @bytes
*
*  @param[in] data
*    Pointer to memory block
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create(const void * data, size_t bytes);

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
*    Returns the size of the value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Size in bytes of the value
*/
REFLECT_API size_t value_size(value v);

/**
*  @brief
*    Increment reference count of a value
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_ref_inc(value v);

/**
*  @brief
*    Decrement reference count of a value
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_ref_dec(value v);

/**
*  @brief
*    Get pointer reference to value data
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Pointer to writeable memory block of value @v
*/
REFLECT_API void * value_data(value v);

/**
*  @brief
*    Convert value @v to memory block @data
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] data
*    Pointer to memory block where value will be stored
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*/
REFLECT_API void value_to(value v, void * data, size_t bytes);

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
*    Assing memory block @data to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] data
*    Pointer to memory block to be copied into @v
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*
*  @return
*    Value with @data of size @bytes assigned to it
*/
REFLECT_API value value_from(value v, const void * data, size_t bytes);

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

/**
*  @brief
*    Destroy a value from scope stack
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_destroy(value v);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_H */
