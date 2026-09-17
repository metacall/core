-- mixed_test.lua
-- Comprehensive edge case test script for Lua loader
-- Tests numeric boundaries, string handling, containers, and error conditions

-- Numeric boundary functions
function lua_mixed_max_int()
	return 9223372036854775807
end

function lua_mixed_min_int()
	return -9223372036854775808
end

function lua_mixed_pi()
	return 3.14159265358979323846
end

function lua_mixed_infinity()
	return math.huge
end

function lua_mixed_nan()
	return 0 / 0
end

-- String handling functions
function lua_mixed_empty_string()
	return ""
end

function lua_mixed_concat(left, right)
	return left .. right
end

function lua_mixed_unicode()
	-- UTF-8 encoded string
	return "Hello World"
end

function lua_mixed_long_string()
	local s = ""
	for i = 1, 1000 do
		s = s .. "x"
	end
	return s
end

-- Container functions
function lua_mixed_empty_table()
	return {}
end

function lua_mixed_simple_array()
	return {1, 2, 3, 4, 5}
end

function lua_mixed_nested_table()
	return {
		name = "test",
		values = {10, 20, 30},
		config = {
			enabled = true,
			count = 42
		}
	}
end

function lua_mixed_get_nested()
	local t = {
		a = {
			b = {
				c = "deep_value"
			}
		}
	}
	return t.a.b.c
end

function lua_mixed_mixed_types()
	return {1, "two", true, nil, 5.5}
end

function lua_mixed_sparse_array()
	return {[1] = 1, [3] = 3, [5] = 5}
end

-- Function variation tests
function lua_mixed_no_args()
	return 42
end

function lua_mixed_return_nil()
	return nil
end

function lua_mixed_no_return()
	local x = 100
end

function lua_mixed_multi_return()
	return 1, 2, 3
end

function lua_mixed_sum_varargs(...)
	local sum = 0
	local args = {...}
	for i = 1, #args do
		sum = sum + args[i]
	end
	return sum
end

function lua_mixed_count_args(...)
	return #{...}
end

function lua_mixed_many_params(a, b, c, d, e, f, g, h, i, j)
	return a + b + c + d + e + f + g + h + i + j
end

-- Type conversion tests
function lua_mixed_type_roundtrip(val)
	return val
end

function lua_mixed_is_nil(val)
	return val == nil
end

function lua_mixed_is_boolean(val)
	return type(val) == "boolean"
end

function lua_mixed_is_number(val)
	return type(val) == "number"
end

function lua_mixed_is_string(val)
	return type(val) == "string"
end

function lua_mixed_is_table(val)
	return type(val) == "table"
end

-- Arithmetic tests
function lua_mixed_add(a, b)
	return a + b
end

function lua_mixed_divide(a, b)
	if b ~= 0 then
		return a / b
	else
		return nil
	end
end

function lua_mixed_factorial(n)
	if n <= 1 then
		return 1
	else
		return n * lua_mixed_factorial(n - 1)
	end
end

-- Table manipulation
function lua_mixed_array_length(arr)
	return #arr
end

function lua_mixed_sum_array(arr)
	local sum = 0
	for i = 1, #arr do
		sum = sum + arr[i]
	end
	return sum
end

function lua_mixed_map_keys(m)
	local keys = {}
	for k, _ in pairs(m) do
		keys[#keys + 1] = k
	end
	return keys
end

-- Boolean logic
function lua_mixed_and(a, b)
	return a and b
end

function lua_mixed_or(a, b)
	return a or b
end

function lua_mixed_not(val)
	return not val
end

-- String operations
function lua_mixed_string_len(s)
	return #s
end

function lua_mixed_substring(s, start_idx, end_idx)
	return string.sub(s, start_idx, end_idx)
end

function lua_mixed_uppercase(s)
	return string.upper(s)
end

function lua_mixed_lowercase(s)
	return string.lower(s)
end

-- Complex data structure
function lua_mixed_get_user()
	return {
		id = 1,
		name = "Test User",
		email = "test@example.com",
		active = true,
		scores = {95, 87, 92},
		metadata = {
			created = "2024-01-01",
			updated = "2024-01-15"
		}
	}
end

function lua_mixed_process_user(user)
	return {
		name_upper = string.upper(user.name or ""),
		score_avg = lua_mixed_sum_array(user.scores or {}) / (#(user.scores or {})),
		is_active = user.active == true
	}
end
