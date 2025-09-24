/*
 *	Format Library by Parra Studios
 *	A cross-platform library for supporting formatted input / output.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_HPP
#define METACALL_HPP 1

/* -- Headers -- */

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace metacall
{
#include <metacall/metacall.h>

class value_base
{
public:
	// Non-copyable, but movable
	value_base(const value_base &) = delete;
	value_base &operator=(const value_base &) = delete;

	value_base(value_base &&) noexcept = default;
	value_base &operator=(value_base &&) noexcept = default;

	// Access the raw value
	void *to_raw() const
	{
		return value_ptr.get();
	}

protected:
	std::unique_ptr<void, void (*)(void *)> value_ptr;

	explicit value_base(void *value_ptr, void (*destructor)(void *) = &metacall_value_destroy) :
		value_ptr(value_ptr, destructor) {}

	static void noop_destructor(void *) {}
};

template <typename T>
class METACALL_API value : public value_base
{
public:
	explicit value(const T &v, void (*destructor)(void *) = &metacall_value_destroy) :
		value_base(create(v), destructor)
	{
		if (value_ptr == nullptr)
		{
			throw std::runtime_error("Failed to create MetaCall value");
		}
	}

	explicit value(void *value_ptr) :
		value_base(value_ptr, &value_base::noop_destructor)
	{
		if (metacall_value_id(value_ptr) != id())
		{
			throw std::runtime_error("Failed to create MetaCall value, the received MetaCall value type does not match with the value class type");
		}
	}

	T to_value() const
	{
		throw std::runtime_error("Unsupported MetaCall value");
	}

private:
	// Type-specific creation (calls specialized version below)
	static void *create(const T &v);

	// Type-specific type id
	static enum metacall_value_id id();
};

template <>
inline void *value<bool>::create(const bool &v)
{
	return metacall_value_create_bool(v);
}

template <>
inline enum metacall_value_id value<bool>::id()
{
	return METACALL_BOOL;
}

template <>
inline bool value<bool>::to_value() const
{
	return metacall_value_to_bool(value_ptr.get());
}

template <>
inline void *value<char>::create(const char &v)
{
	return metacall_value_create_char(v);
}

template <>
inline enum metacall_value_id value<char>::id()
{
	return METACALL_CHAR;
}

template <>
inline char value<char>::to_value() const
{
	return metacall_value_to_char(value_ptr.get());
}

template <>
inline void *value<short>::create(const short &v)
{
	return metacall_value_create_short(v);
}

template <>
inline enum metacall_value_id value<short>::id()
{
	return METACALL_SHORT;
}

template <>
inline short value<short>::to_value() const
{
	return metacall_value_to_short(value_ptr.get());
}

template <>
inline void *value<int>::create(const int &v)
{
	return metacall_value_create_int(v);
}

template <>
inline enum metacall_value_id value<int>::id()
{
	return METACALL_INT;
}

template <>
inline int value<int>::to_value() const
{
	return metacall_value_to_int(value_ptr.get());
}

template <>
inline void *value<long>::create(const long &v)
{
	return metacall_value_create_long(v);
}

template <>
inline enum metacall_value_id value<long>::id()
{
	return METACALL_LONG;
}

template <>
inline long value<long>::to_value() const
{
	return metacall_value_to_long(value_ptr.get());
}

template <>
inline void *value<float>::create(const float &v)
{
	return metacall_value_create_float(v);
}

template <>
inline enum metacall_value_id value<float>::id()
{
	return METACALL_FLOAT;
}

template <>
inline float value<float>::to_value() const
{
	return metacall_value_to_float(value_ptr.get());
}

template <>
inline void *value<double>::create(const double &v)
{
	return metacall_value_create_double(v);
}

template <>
inline enum metacall_value_id value<double>::id()
{
	return METACALL_DOUBLE;
}

template <>
inline double value<double>::to_value() const
{
	return metacall_value_to_double(value_ptr.get());
}

template <>
inline void *value<std::string>::create(const std::string &v)
{
	return metacall_value_create_string(v.c_str(), v.size());
}

template <>
inline enum metacall_value_id value<std::string>::id()
{
	return METACALL_STRING;
}

template <>
inline std::string value<std::string>::to_value() const
{
	return metacall_value_to_string(value_ptr.get());
}

template <>
inline void *value<const char *>::create(const char *const &v)
{
	return metacall_value_create_string(v, std::strlen(v));
}

template <>
inline enum metacall_value_id value<const char *>::id()
{
	return METACALL_STRING;
}

template <>
inline const char *value<const char *>::to_value() const
{
	return metacall_value_to_string(value_ptr.get());
}

template <>
inline void *value<std::vector<char>>::create(const std::vector<char> &v)
{
	return metacall_value_create_buffer(v.data(), v.size());
}

template <>
inline enum metacall_value_id value<std::vector<char>>::id()
{
	return METACALL_BUFFER;
}

template <>
inline std::vector<char> value<std::vector<char>>::to_value() const
{
	void *ptr = value_ptr.get();
	char *buffer = static_cast<char *>(metacall_value_to_buffer(ptr));
	std::vector<char> buffer_vector(buffer, buffer + metacall_value_count(ptr));

	return buffer_vector;
}

template <>
inline void *value<std::vector<unsigned char>>::create(const std::vector<unsigned char> &v)
{
	return metacall_value_create_buffer(v.data(), v.size());
}

template <>
inline enum metacall_value_id value<std::vector<unsigned char>>::id()
{
	return METACALL_BUFFER;
}

template <>
inline std::vector<unsigned char> value<std::vector<unsigned char>>::to_value() const
{
	void *ptr = value_ptr.get();
	unsigned char *buffer = static_cast<unsigned char *>(metacall_value_to_buffer(ptr));
	std::vector<unsigned char> buffer_vector(buffer, buffer + metacall_value_count(ptr));

	return buffer_vector;
}

template <>
inline void *value<void *>::create(void *const &v)
{
	return metacall_value_create_ptr(v);
}

template <>
inline enum metacall_value_id value<void *>::id()
{
	return METACALL_PTR;
}

template <>
inline void *value<void *>::to_value() const
{
	return metacall_value_to_ptr(value_ptr.get());
}

template <>
inline void *value<std::nullptr_t>::create(const std::nullptr_t &)
{
	return metacall_value_create_null();
}

template <>
inline enum metacall_value_id value<std::nullptr_t>::id()
{
	return METACALL_NULL;
}

template <>
inline std::nullptr_t value<std::nullptr_t>::to_value() const
{
	return nullptr;
}

// TODO: Array, Map, Future, Function, Class, Object, Exception, Throwable...

template <typename K, typename V>
class METACALL_API map : public value_base
{
public:
	using pair_type = std::pair<K, V>;
	using pair_value_type = std::pair<value<K>, value<V>>;

	map(std::initializer_list<pair_type> list) :
		value_base(metacall_value_create_map(NULL, list.size()))
	{
		if (value_ptr == nullptr)
		{
			throw std::runtime_error("Failed to create MetaCall map value");
		}

		void **map_array = metacall_value_to_map(value_ptr.get());
		size_t index = 0;

		for (const auto &pair : list)
		{
			void *tuple = metacall_value_create_array(nullptr, 2);
			void **tuple_array = metacall_value_to_array(tuple);

			// Create the pair
			auto value_pair = std::make_pair(value<K>(pair.first, &value_base::noop_destructor), value<V>(pair.second, &value_base::noop_destructor));

			// Insert into metacall value map
			tuple_array[0] = value_pair.first.to_raw();
			tuple_array[1] = value_pair.second.to_raw();

			map_array[index++] = tuple;

			// Store into the map
			m.emplace(pair.first, std::move(value_pair));
		}
	}

	explicit map(void *value_ptr) :
		value_base(value_ptr, &value_base::noop_destructor)
	{
		if (metacall_value_id(value_ptr) != METACALL_MAP)
		{
			throw std::runtime_error("MetaCall map initialized with a MetaCall value which is not of type map");
		}

		rehash();
	}

	void rehash()
	{
		void *ptr = value_ptr.get();
		void **map_array = metacall_value_to_map(ptr);
		const size_t size = metacall_value_count(ptr);

		m.clear();

		for (size_t index = 0; index < size; ++index)
		{
			void **tuple_array = metacall_value_to_array(map_array[index]);

			// Create the values
			auto pair = std::make_pair(value<K>(tuple_array[0]), value<V>(tuple_array[1]));

			// Store into the map
			m.emplace(pair.first.to_value(), std::move(pair));
		}
	}

	V operator[](const K &key) const
	{
		return m.at(key).second.to_value();
	}

private:
	/*
	// Case 1: value is value_base (e.g. nested metacall map value)
	template <typename T>
	static typename std::enable_if<std::is_base_of<value_base, T>::value, value<T>>::type
	wrap_value(const T &v)
	{
		return const_cast<T &>(v);
	}

	// Case 2: value is a plain type
	template <typename T>
	static typename std::enable_if<!std::is_base_of<value_base, T>::value, value<T>>::type
	wrap_value(const T &v)
	{
		value<T> val(v);
		return val;
	}
	*/

	std::unordered_map<K, pair_value_type> m;
};

template <typename... Ts>
METACALL_API int metacall(std::string name, Ts... ts);

} /* namespace metacall */

// TODO: Move everything to metacall.inl

#include <metacall/metacall.inl>

#endif /* METACALL_HPP */
