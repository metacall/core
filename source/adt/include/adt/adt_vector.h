/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_VECTOR_H
#define ADT_VECTOR_H 1

/* -- Headers -- */

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct vector_type;

/* -- Type Definitions -- */

typedef struct vector_type *vector;

/* -- Macros -- */

/**
 *  @brief
 *    Create a vector in memory
 *
 *  @param[in] type_name
 *    Type of each element of the vector
 *
 *  @return
 *    A pointer to the vector memory block
 */
#define vector_create_type(type_name) \
	vector_create(sizeof(type_name))

/**
 *  @brief
 *    Create a vector with @capacity elements preallocated
 *
 *  @param[in] type_name
 *    Type of each element of the vector
 *
 *  @param[in] capacity
 *    Preallocated elements of the vector
 *
 *  @return
 *    A pointer to the vector memory block
 */
#define vector_create_reserve_type(type_name, capacity) \
	vector_create_reserve(sizeof(type_name), capacity)

/**
 *  @brief
 *    Access to the first element in vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] type_name
 *    Type of element to be accessed
 *
 *  @return
 *    Pointer to the first element
 */
#define vector_front_type(v, type_name) \
	(*((type_name *)vector_front(v)))

/**
 *  @brief
 *    Access to the last element in vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] type_name
 *    Type of element to be accessed
 *
 *  @return
 *    Pointer to the last element
 */
#define vector_back_type(v, type_name) \
	(*((type_name *)vector_back(v)))

/**
 *  @brief
 *    Access to object at @position where:
 *    	[0 <= @position <= vector_size(v) - 1 ]
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be accessed
 *
 *  @param[in] type_name
 *    Type of element to be accessed
 *
 *  @return
 *    Pointer to the element at @position
 */
#define vector_at_type(v, position, type_name) \
	(*((type_name *)vector_at(v, position)))

/**
 *  @brief
 *    Sets a variable object at @position to the
 *    same object pointed by @element
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be modified
 *
 *  @param[in] variable
 *    Variable element to be modified
 *
 *  @param[in] type_name
 *    Type of element to be modified
 */
#define vector_set_var(v, position, variable) \
	vector_set(v, position, &variable)

/**
 *  @brief
 *    Adds a variable object at the end of vector
 *    (increments allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] variable
 *    Variable element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_push_back_var(v, variable) \
	vector_push_back(v, &variable)

/**
 *  @brief
 *    Adds a variable object in first position of vector
 *    (increments allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] variable
 *    Variable element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_push_front_var(v, variable) \
	vector_push_front(v, &variable)

/**
 *  @brief
 *    Adds a variable object in @position where:
 *    	[ 0 <= @position <= vector_size(v) - 1 ]
 *    Tail objects from @position will be moved (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be inserted
 *
 *  @param[in] variable
 *    Variable element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_insert_var(v, position, variable) \
	vector_insert(v, position, &variable)

/**
 *  @brief
 *    Sets a constant object at @position to the
 *    same object pointed by @element
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be modified
 *
 *  @param[in] constant
 *    Constant element to be modified
 *
 *  @param[in] type_name
 *    Type of element to be modified
 */
#define vector_set_const(v, position, constant, type_name)        \
	do                                                            \
	{                                                             \
		type_name macro_vector_type_const_to_var = constant;      \
                                                                  \
		vector_set(v, position, &macro_vector_type_const_to_var); \
                                                                  \
	} while (0)

/**
 *  @brief
 *    Adds a constant object at the end of vector
 *    (increments allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] constant
 *    Constant element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_push_back_const(v, constant, type_name)        \
	do                                                        \
	{                                                         \
		type_name macro_vector_type_const_to_var = constant;  \
                                                              \
		vector_push_back(v, &macro_vector_type_const_to_var); \
                                                              \
	} while (0)

/**
 *  @brief
 *    Adds a constant object in first position of vector
 *    (increments allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] constant
 *    Constant element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_push_front_const(v, constant, type_name)        \
	do                                                         \
	{                                                          \
		type_name macro_vector_type_const_to_var = constant;   \
                                                               \
		vector_push_front(v, &macro_vector_type_const_to_var); \
                                                               \
	} while (0)

/**
 *  @brief
 *    Adds a constant object in @position where:
 *    	[ 0 <= @position <= vector_size(v) - 1 ]
 *    Tail objects from @position will be moved (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be inserted
 *
 *  @param[in] constant
 *    Constant element to be inserted
 *
 *  @param[in] type_name
 *    Type of element to be inserted
 */
#define vector_insert_const(v, position, constant, type_name)        \
	do                                                               \
	{                                                                \
		type_name macro_vector_type_const_to_var = constant;         \
                                                                     \
		vector_insert(v, position, &macro_vector_type_const_to_var); \
                                                                     \
	} while (0)

/* -- Methods -- */

/**
 *  @brief
 *    Create a vector in memory
 *
 *  @param[in] type_size
 *    Size of the element type vector will allocate
 *
 *  @return
 *    A pointer to the vector memory block
 */
ADT_API vector vector_create(size_t type_size);

/**
 *  @brief
 *    Create a vector with @capacity elements preallocated
 *
 *  @param[in] type_size
 *    Size of the element type vector will allocate
 *
 *  @param[in] capacity
 *    Preallocated elements of the vector
 *
 *  @return
 *    A pointer to the vector memory block
 */
ADT_API vector vector_create_reserve(size_t type_size, size_t capacity);

/**
 *  @brief
 *    Copy a vector
 *
 *  @param[in] v
 *    Source vector to be copied
 *
 *  @return
 *    A pointer to a vector memory block copy
 */
ADT_API vector vector_copy(vector v);

/**
 *  @brief
 *    Request a change in vector capacity
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] capacity
 *    New capacity of vector
 *
 *  @return
 *    Positive number on error, zero on success
 */
ADT_API int vector_reserve(vector v, size_t capacity);

/**
 *  @brief
 *    Resizes container so that it contains @size elements
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] size
 *    New size of vector
 *
 *  @return
 *    Positive number on error, zero on success
 */
ADT_API int vector_resize(vector v, size_t size);

/**
 *  @brief
 *    Retreive amount of objects can be stored
 *    with current allocated memory
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Capacity of vector
 */
ADT_API size_t vector_capacity(vector v);

/**
 *  @brief
 *    Retreive amount of current elements in vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Current elements in vector
 */
ADT_API size_t vector_size(vector v);

/**
 *  @brief
 *    Retreive size of element type of vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Size of element type
 */
ADT_API size_t vector_type_size(vector v);

/**
 *  @brief
 *    Access to the first element in vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Pointer to the first element
 */
ADT_API void *vector_front(vector v);

/**
 *  @brief
 *    Access to the last element in vector
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Pointer to the last element
 */
ADT_API void *vector_back(vector v);

/**
 *  @brief
 *    Access to object at @position where:
 *    	[0 <= @position <= vector_size(v) - 1 ]
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be accessed
 *
 *  @return
 *    Pointer to the element at @position
 */
ADT_API void *vector_at(vector v, size_t position);

/**
 *  @brief
 *    Sets the object at @position to the same object
 *    pointed by @element
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be modified
 *
 *  @param[in] element
 *    Element to be copied at @position
 */
ADT_API void vector_set(vector v, size_t position, void *element);

/**
 *  @brief
 *    Adds an object at the end of vector without initializing
 *    it, all objects be moved (increments allocated memory if
 *    neccessary; new element could contain garbage)
 *
 *  @param[in] v
 *    Vector pointer
 */
ADT_API void vector_push_back_empty(vector v);

/**
 *  @brief
 *    Adds an object at the end of vector (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] element
 *    Element to be inserted
 */
ADT_API void vector_push_back(vector v, void *element);

/**
 *  @brief
 *    Deletes the last object in vector (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 */
ADT_API void vector_pop_back(vector v);

/**
 *  @brief
 *    Adds an object in first position without initializing
 *    it, all objects be moved (increments allocated memory
 *    if neccessary; new element could contain garbage)
 *
 *  @param[in] v
 *    Vector pointer
 */
ADT_API void vector_push_front_empty(vector v);

/**
 *  @brief
 *    Adds an object in first position of vector (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] element
 *    Element to be inserted
 */
ADT_API void vector_push_front(vector v, void *element);

/**
 *  @brief
 *    Deletes the first object in vector (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 */
ADT_API void vector_pop_front(vector v);

/**
 *  @brief
 *    Adds an object in @position without initializing
 *    it where:
 *    	[ 0 <= @position <= vector_size(v) - 1 ]
 *    Tail objects from @position will be moved (increments
 *    allocated memory if neccessary; new element could
 *    contain garbage)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be inserted
 */
ADT_API void vector_insert_empty(vector v, size_t position);

/**
 *  @brief
 *    Adds an object in @position where:
 *    	[ 0 <= @position <= vector_size(v) - 1 ]
 *    Tail objects from @position will be moved (increments
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be inserted
 *
 *  @param[in] element
 *    Reference to the element to be inserted
 */
ADT_API void vector_insert(vector v, size_t position, void *element);

/**
 *  @brief
 *    Deletes an object in @position (decrements
 *    allocated memory if neccessary)
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @param[in] position
 *    Position of element to be erased
 */
ADT_API void vector_erase(vector v, size_t position);

/**
 *  @brief
 *    Clear internal data of the vector and set
 *    size to zero
 *
 *  @param[in] v
 *    Vector pointer
 *
 *  @return
 *    Positive number on error, zero on success
 */
ADT_API int vector_clear(vector v);

/**
 *  @brief
 *    Destroy a vector from memory (do not apply
 *    same operation twice over same object)
 *
 *  @param[in] v
 *    Vector pointer
 */
ADT_API void vector_destroy(vector v);

#ifdef __cplusplus
}
#endif

#endif /* ADT_VECTOR_H */
