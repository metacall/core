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

#include <array>
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

	// Release the unique pointer and return its contents
	void *release()
	{
		return value_ptr.release();
	}

protected:
	std::unique_ptr<void, void (*)(void *)> value_ptr;

	explicit value_base(void *value_ptr, void (*destructor)(void *) = &metacall_value_destroy) :
		value_ptr(value_ptr, destructor) {}

	static void noop_destructor(void *) {}
};

template <typename T>
class value : public value_base
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

	explicit value(void *value_ptr, void (*destructor)(void *) = &value_base::noop_destructor) :
		value_base(value_ptr, destructor)
	{
		if (metacall_value_id(value_ptr) != id())
		{
			throw std::runtime_error("Failed to create MetaCall value, the received MetaCall value type does not match with the value class type");
		}
	}

	T to_value() const &
	{
		throw std::runtime_error("Unsupported MetaCall value");
	}

	// Type-specific creation (calls specialized version below)
	template <typename U = T, typename = std::enable_if_t<!std::is_base_of_v<value_base, U>>>
	static void *create(const U &v);

	template <typename U = T, typename = std::enable_if_t<std::is_base_of_v<value_base, U>>>
	static void *create(U &v);

	// Type-specific type id
	static enum metacall_value_id id();
};

template <>
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
inline bool value<bool>::to_value() const &
{
	return metacall_value_to_bool(to_raw());
}

template <>
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
inline char value<char>::to_value() const &
{
	return metacall_value_to_char(to_raw());
}

template <>
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
inline short value<short>::to_value() const &
{
	return metacall_value_to_short(to_raw());
}

template <>
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
inline int value<int>::to_value() const &
{
	return metacall_value_to_int(to_raw());
}

template <>
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
inline long value<long>::to_value() const &
{
	return metacall_value_to_long(to_raw());
}

template <>
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
inline float value<float>::to_value() const &
{
	return metacall_value_to_float(to_raw());
}

template <>
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
inline double value<double>::to_value() const &
{
	return metacall_value_to_double(to_raw());
}

template <>
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
inline std::string value<std::string>::to_value() const &
{
	return metacall_value_to_string(to_raw());
}

template <>
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
inline const char *value<const char *>::to_value() const &
{
	return metacall_value_to_string(to_raw());
}

template <>
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
inline std::vector<char> value<std::vector<char>>::to_value() const &
{
	void *ptr = to_raw();
	char *buffer = static_cast<char *>(metacall_value_to_buffer(ptr));
	std::vector<char> buffer_vector(buffer, buffer + metacall_value_count(ptr));

	return buffer_vector;
}

template <>
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
inline std::vector<unsigned char> value<std::vector<unsigned char>>::to_value() const &
{
	void *ptr = to_raw();
	unsigned char *buffer = static_cast<unsigned char *>(metacall_value_to_buffer(ptr));
	std::vector<unsigned char> buffer_vector(buffer, buffer + metacall_value_count(ptr));

	return buffer_vector;
}

template <>
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
inline void *value<void *>::to_value() const &
{
	return metacall_value_to_ptr(to_raw());
}

template <>
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
inline std::nullptr_t value<std::nullptr_t>::to_value() const &
{
	return nullptr;
}

// TODO: Future, Function, Class, Object, Exception, Throwable...

class value_ref
{
public:
	explicit value_ref(void *ptr) :
		ptr(ptr) {}

	template <typename T>
	T as() const
	{
		return value<T>(ptr).to_value();
	}

private:
	void *ptr;
};

class array : public value_base
{
public:
	explicit array(void *array_value) :
		value_base(array_value, &value_base::noop_destructor)
	{
	}

	template <typename... Args>
	explicit array(Args &&...args) :
		value_base(create(std::forward<Args>(args)...), &metacall_value_destroy)
	{
		if (value_ptr == nullptr)
		{
			throw std::runtime_error("Failed to create MetaCall array");
		}
	}

	array(array &arr) :
		value_base(arr.to_raw(), &value_base::noop_destructor) {}

	array(array &&arr) noexcept :
		value_base(arr.value_ptr.release(), arr.value_ptr.get_deleter()) {}

	template <typename T>
	T get(std::size_t index) const
	{
		void **array_ptr = to_array();

		return value<T>(array_ptr[index]).to_value();
	}

	value_ref operator[](std::size_t index) const
	{
		void **array_ptr = to_array();

		return value_ref(array_ptr[index]);
	}

	static enum metacall_value_id id()
	{
		return METACALL_ARRAY;
	}

private:
	void **to_array() const
	{
		void **array_ptr = metacall_value_to_array(to_raw());

		if (array_ptr == NULL)
		{
			throw std::runtime_error("Invalid MetaCall array");
		}

		return array_ptr;
	}

	// Recursive function to create and fill the MetaCall array
	template <typename... Args>
	static void *create(Args &&...args)
	{
		constexpr std::size_t size = sizeof...(Args);

		// Create the array with null data initially
		void *array_value = metacall_value_create_array(NULL, size);

		if (array_value == NULL)
		{
			throw std::runtime_error("Failed to create MetaCall value array");
		}

		// Get the internal C array
		void **array_ptr = metacall_value_to_array(array_value);

		// Helper to unpack the args into array
		create_array(array_ptr, 0, std::forward<Args>(args)...);

		return array_value;
	}

	// Recursive unpacking using fold expression (C++17+)
	template <typename... Args>
	static void create_array(void **array_ptr, std::size_t index, Args &&...args)
	{
		// Use initializer list trick to expand the pack
		((
			 array_ptr[index++] = value<std::decay_t<Args>>::create(std::forward<Args>(args))),
			...);
	}
};

template <>
template <>
inline void *value<array>::create(array &v)
{
	return v.release();
}

template <>
inline enum metacall_value_id value<array>::id()
{
	return METACALL_ARRAY;
}

template <>
inline array value<array>::to_value() const &
{
	return array(to_raw());
}

template <typename K, typename V>
class map : public value_base
{
public:
	using key_type = K;
	using value_type = V;
	using pair_type = std::pair<K, V>;
	using pair_value_type = std::pair<value<K>, value<V>>;

	map(std::initializer_list<pair_type> list) :
		value_base(metacall_value_create_map(NULL, list.size()))
	{
		if (value_ptr == nullptr)
		{
			throw std::runtime_error("Failed to create MetaCall map value");
		}

		void **map_array = metacall_value_to_map(to_raw());
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

	V operator[](const K &key) const
	{
		return m.at(key).second.to_value();
	}

	static enum metacall_value_id id()
	{
		return METACALL_MAP;
	}

protected:
	void rehash()
	{
		void *ptr = to_raw();
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

private:
	std::unordered_map<K, pair_value_type> m;
};

// Partial specialization of map for value
template <typename K, typename V>
class value<map<K, V>> : public value_base
{
public:
	using map_type = map<K, V>;

	explicit value(void *ptr, void (*destructor)(void *) = &metacall_value_destroy) :
		value_base(ptr, destructor) {}

	static void *create(map_type &v)
	{
		return v.release();
	}

	static enum metacall_value_id id()
	{
		return METACALL_MAP;
	}

	map_type to_value() const &
	{
		return map_type(to_raw());
	}
};

namespace detail
{
template <typename T>
constexpr bool is_value_base_v = std::is_base_of_v<value_base, std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename T>
struct is_array : std::false_type
{
};

template <>
struct is_array<metacall::array> : std::true_type
{
};

template <typename T>
inline constexpr bool is_array_v = is_array<std::remove_cv_t<std::remove_reference_t<T>>>::value;

template <typename T>
struct is_map : std::false_type
{
};

template <typename K, typename V>
struct is_map<metacall::map<K, V>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_map_v = is_map<std::remove_cv_t<std::remove_reference_t<T>>>::value;

template <typename T>
value_base to_value_base(T &&arg)
{
	if constexpr (is_value_base_v<T>)
	{
		return std::move(arg);
	}
	else
	{
		return value<std::decay_t<T>>(std::forward<T>(arg));
	}
}

template <typename Arg>
auto arg_to_value(void *arg)
{
	if constexpr (is_array_v<Arg>)
	{
		return metacall::array(arg);
	}
	else if constexpr (is_map_v<Arg>)
	{
		return metacall::map<typename Arg::key_type, typename Arg::value_type>(arg);
	}
	else
	{
		return metacall::value<Arg>(arg).to_value();
	}
}

template <typename... Args, auto... Is>
auto register_function_args_tuple(void *args[], std::index_sequence<Is...>)
{
	return std::tuple{ arg_to_value<std::remove_cv_t<std::remove_reference_t<Args>>>(args[Is])... };
}

template <typename... Args>
auto register_function_args(void *args[])
{
	return register_function_args_tuple<Args...>(args, std::make_index_sequence<sizeof...(Args)>());
}

template <typename Ret>
int register_function(const char *name, Ret (*func)(void), void **func_ptr)
{
	auto invoke = [](size_t argc, void *[], void *data) -> void * {
		// Check for correct argument size
		if (argc != 0)
		{
			// TODO: This must be: return metacall::value<error>
			throw std::invalid_argument(
				"Incorrect number of arguments. Expected no arguments, received " +
				std::to_string(argc) +
				" arguments.");
		}

		// Get target function from closure
		auto func = (Ret(*)(void))(data);

		// Execute the call
		auto result = func();

		// Generate return value
		return value<Ret>::create(result);
	};

	enum metacall_value_id types[] = { METACALL_INVALID };

	return metacall::metacall_registerv_closure(
		name,
		invoke,
		func_ptr,
		value<std::remove_cv_t<std::remove_reference_t<Ret>>>::id(),
		0,
		types,
		(void *)(func));
}

template <typename Ret, typename... Args>
int register_function(const char *name, Ret (*func)(Args...), void **func_ptr)
{
	auto invoke = [](size_t argc, void *args[], void *data) -> void * {
		// Check for correct argument size
		if (argc != sizeof...(Args))
		{
			// TODO: This must be: return metacall::value<error>
			throw std::invalid_argument(
				"Incorrect number of arguments. Expected " +
				std::to_string(sizeof...(Args)) +
				" arguments, received " +
				std::to_string(argc) +
				" arguments.");
		}

		// Convert arguments from the void* array to a typed tuple of metacall values
		auto tuple_args = register_function_args<Args...>(args);

		// Get target function from closure
		auto func = (Ret(*)(Args...))(data);

		// Apply the function to the unpacked arguments
		auto result = std::apply(func, tuple_args);

		// Generate return value
		return value<Ret>::create(result);
	};

	enum metacall_value_id types[] = { value<std::remove_cv_t<std::remove_reference_t<Args>>>::id()... };

	return metacall::metacall_registerv_closure(
		name,
		invoke,
		func_ptr,
		value<std::remove_cv_t<std::remove_reference_t<Ret>>>::id(),
		sizeof...(Args),
		types,
		(void *)(func));
}

inline void **null_safe_args(void *args[])
{
	return args ? args : metacall_null_args;
}

} /* namespace detail */

template <typename Ret, typename... Args>
class function
{
public:
	explicit function(void *func) :
		func(func) {}

	~function() {}

	value<Ret> operator()(Args &&...args) const
	{
		constexpr std::size_t size = sizeof...(Args);
		std::array<value_base, size> value_args = { { detail::to_value_base(std::forward<Args>(args))... } };
		std::array<void *, size> raw_args;

		for (std::size_t i = 0; i < size; ++i)
		{
			raw_args[i] = value_args[i].to_raw();
		}

		void *ret = metacallfv_s(func, detail::null_safe_args(raw_args.data()), size);

		if (ret == NULL)
		{
			throw std::runtime_error("MetaCall invokation has failed by returning NULL");
		}

		return value<Ret>(ret, &metacall_value_destroy);
	}

private:
	void *func;
};

template <typename Ret, typename... Args>
void register_function(const char *name, Ret (*func)(Args...), void **func_ptr = nullptr)
{
	if (detail::register_function(name, func, func_ptr) != 0)
	{
		throw std::runtime_error("Function '" + std::string(name) + "' failed to be registered.");
	}
}

template <typename Ret, typename... Args>
function<Ret, Args...> register_function(Ret (*func)(Args...))
{
	void *func_ptr = nullptr;

	if (detail::register_function(NULL, func, &func_ptr) != 0)
	{
		throw std::runtime_error("Function failed to be registered.");
	}

	return function<Ret, Args...>(func_ptr);
}

template <typename Ret, typename... Args>
value<Ret> metacall(std::string name, Args &&...args)
{
	constexpr std::size_t size = sizeof...(Args);
	std::array<value_base, size> value_args = { { detail::to_value_base(std::forward<Args>(args))... } };
	std::array<void *, size> raw_args;

	for (std::size_t i = 0; i < size; ++i)
	{
		raw_args[i] = value_args[i].to_raw();
	}

	void *ret = metacallv_s(name.c_str(), null_safe_args(raw_args.data()), size);

	if (ret == NULL)
	{
		throw std::runtime_error("MetaCall invokation to '" + name + "' has failed by returning NULL");
	}

	return value<Ret>(ret, &metacall_value_destroy);
}

} /* namespace metacall */

// TODO: Move implementations to metacall.inl
#include <metacall/metacall.inl>

#endif /* METACALL_HPP */
