/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

/* -- Headers -- */

#include <adt/adt_vector.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define VECTOR_CAPACITY_INCREMENT 2	 /**< Capacity increment */
#define VECTOR_CAPACITY_MIN		  16 /**< Minimum capacity */
#define VECTOR_CAPACITY_MIN_USED  8	 /**< Max unused capacity since free memory */

/* -- Member Data -- */

struct vector_type
{
	size_t type_size; /**< Size of element type */
	size_t capacity;  /**< Allocated capacity of the vector */
	size_t size;	  /**< Amount of actual elements cointained in vector */
	void *data;		  /**< Pointer to memory block */
};

/* -- Private Methods -- */

/**
*  @brief
*    Vector access by offset bytes
*
*  @param[in] v
*    Vector pointer
*
*  @param[in] bytes
*    Offset memory in bytes
*
*  @return
*    A pointer to data vector memory block with offset @bytes
*/
static void *vector_data_offset_bytes(vector v, size_t bytes);

/* -- Methods -- */

void *vector_data_offset_bytes(vector v, size_t bytes)
{
	if (v != NULL && v->data != NULL && bytes < v->capacity * v->type_size)
	{
		return ((void *)(((char *)v->data) + (bytes)));
	}

	return NULL;
}

vector vector_create(size_t type_size)
{
	return vector_create_reserve(type_size, VECTOR_CAPACITY_MIN);
}

vector vector_create_reserve(size_t type_size, size_t capacity)
{
	if (type_size > 0)
	{
		vector v = malloc(sizeof(struct vector_type));

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Vector allocation error");

			return NULL;
		}

		v->type_size = type_size;
		v->capacity = (capacity < VECTOR_CAPACITY_MIN) ? VECTOR_CAPACITY_MIN : capacity;
		v->size = 0;
		v->data = malloc(v->capacity * v->type_size);

		if (v->data == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Vector internal allocation error");

			free(v);

			return NULL;
		}

		return v;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Vector invalid type size");

	return NULL;
}

vector vector_copy(vector v)
{
	if (v != NULL)
	{
		vector dest = malloc(sizeof(struct vector_type));

		if (dest == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Vector allocation error");

			return NULL;
		}

		dest->data = malloc(v->type_size * v->capacity);

		if (dest->data == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Vector internal allocation error");

			free(dest);

			return NULL;
		}

		dest->capacity = v->capacity;
		dest->size = v->size;
		dest->type_size = v->type_size;

		memcpy(dest->data, v->data, dest->size * dest->type_size);

		return dest;
	}

	return NULL;
}

int vector_reserve(vector v, size_t capacity)
{
	if (v != NULL && capacity != v->capacity)
	{
		register void *data;

		if (capacity < VECTOR_CAPACITY_MIN)
		{
			capacity = VECTOR_CAPACITY_MIN;
		}

		data = realloc(v->data, capacity * v->type_size);

		if (data == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Vector reallocation error");

			return 1;
		}

		if (capacity < v->size)
		{
			v->size = capacity;
		}

		v->data = data;

		v->capacity = capacity;

		return 0;
	}

	return 1;
}

int vector_resize(vector v, size_t size)
{
	if (v != NULL)
	{
		if (v->capacity < size)
		{
			if (vector_reserve(v, size) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");

				return 1;
			}
		}

		v->size = size;

		if (v->size < v->capacity / VECTOR_CAPACITY_MIN_USED)
		{
			return vector_reserve(v, v->size * VECTOR_CAPACITY_INCREMENT);
		}

		return 0;
	}

	return 1;
}

size_t vector_capacity(vector v)
{
	if (v != NULL)
	{
		return v->capacity;
	}

	return 0;
}

size_t vector_size(vector v)
{
	if (v != NULL)
	{
		return v->size;
	}

	return 0;
}

size_t vector_type_size(vector v)
{
	if (v != NULL)
	{
		return v->type_size;
	}

	return 0;
}

void *vector_front(vector v)
{
	if (v != NULL)
	{
		return v->data;
	}

	return NULL;
}

void *vector_back(vector v)
{
	if (v != NULL)
	{
		return vector_data_offset_bytes(v, (v->size - 1) * v->type_size);
	}

	return NULL;
}

void *vector_at(vector v, size_t position)
{
	if (v != NULL)
	{
		return vector_data_offset_bytes(v, position * v->type_size);
	}

	return NULL;
}

void vector_set(vector v, size_t position, void *element)
{
	if (v != NULL && position < v->capacity && element != NULL)
	{
		memcpy(vector_data_offset_bytes(v, position * v->type_size), element, v->type_size);
	}
}

void vector_push_back_empty(vector v)
{
	if (v != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");

				return;
			}
		}

		++v->size;
	}
}

void vector_push_back(vector v, void *element)
{
	if (v != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");

				return;
			}
		}

		memcpy(vector_data_offset_bytes(v, v->size * v->type_size), element, v->type_size);

		++v->size;
	}
}

void vector_pop_back(vector v)
{
	if (v != NULL && v->size > 0)
	{
		--v->size;

		if (v->capacity / VECTOR_CAPACITY_MIN_USED < v->size)
		{
			if (vector_reserve(v, v->size * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}
	}
}

void vector_push_front_empty(vector v)
{
	if (v != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}

		memmove(vector_data_offset_bytes(v, v->type_size), v->data, v->size * v->type_size);

		++v->size;
	}
}

void vector_push_front(vector v, void *element)
{
	if (v != NULL && element != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}

		memmove(vector_data_offset_bytes(v, v->type_size), v->data, v->size * v->type_size);

		memcpy(v->data, element, v->type_size);

		++v->size;
	}
}

void vector_pop_front(vector v)
{
	if (v != NULL && v->size > 0)
	{
		--v->size;

		memmove(v->data, vector_data_offset_bytes(v, v->type_size), v->size * v->type_size);

		if (v->capacity / VECTOR_CAPACITY_MIN_USED < v->size)
		{
			if (vector_reserve(v, v->size * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}
	}
}

void vector_insert_empty(vector v, size_t position)
{
	if (v != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}

		if (position < v->size)
		{
			memmove(vector_data_offset_bytes(v, position * v->type_size),
				vector_data_offset_bytes(v, (position + 1) * v->type_size),
				(v->size - position) * v->type_size);
		}

		++v->size;
	}
}

void vector_insert(vector v, size_t position, void *element)
{
	if (v != NULL)
	{
		if (v->size == v->capacity)
		{
			if (vector_reserve(v, v->capacity * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}

		if (position < v->size)
		{
			memmove(vector_data_offset_bytes(v, (position + 1) * v->type_size),
				vector_data_offset_bytes(v, position * v->type_size),
				(v->size - position) * v->type_size);

			memcpy(vector_data_offset_bytes(v, position * v->type_size), element, v->type_size);
		}
		else
		{
			memcpy(vector_data_offset_bytes(v, v->size * v->type_size), element, v->type_size);
		}

		++v->size;
	}
}

void vector_erase(vector v, size_t position)
{
	if (v != NULL && position < v->size)
	{
		if (position < v->size - 1)
		{
			memmove(vector_data_offset_bytes(v, position * v->type_size),
				vector_data_offset_bytes(v, (position + 1) * v->type_size),
				(v->size - position - 1) * v->type_size);
		}

		--v->size;

		if (v->capacity / VECTOR_CAPACITY_MIN_USED < v->size)
		{
			if (vector_reserve(v, v->size * VECTOR_CAPACITY_INCREMENT) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Vector reserve error");
			}
		}
	}
}

int vector_clear(vector v)
{
	return vector_resize(v, 0);
}

void vector_destroy(vector v)
{
	if (v != NULL)
	{
		if (v->data != NULL)
		{
			free(v->data);
		}

		free(v);
	}
}
