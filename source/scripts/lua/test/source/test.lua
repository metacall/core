-- Test script for Lua loader
-- Contains simple functions for testing type handling, arrays, and error conditions

-- Function returning multiple types for testing
function lua_test_types(int_val, double_val, str_val, bool_val)
	return int_val + double_val, str_val, bool_val
end

-- String concatenation test
function lua_test_concat_strings(left, right)
	return left .. right
end

-- Boolean negation
function lua_test_negate_bool(val)
	return not val
end

-- Sum array elements
function lua_test_sum_array(arr)
	local sum = 0
	for i = 1, #arr do
		sum = sum + arr[i]
	end
	return sum
end

-- Return a simple array
function lua_test_get_array()
	return {1, 2, 3, 4, 5}
end

-- Return a table/dict
function lua_test_get_dict()
	return {hello = "world", num = 42, flag = true}
end

-- Error throwing function
function lua_test_throw_error()
	error("Intentional test error")
end

-- Empty function
function lua_test_empty()
	return nil
end

-- Echo function for testing pass-through
function lua_test_echo(val)
	return val
end

-- Calculate factorial
function lua_test_factorial(n)
	if n <= 1 then
		return 1
	else
		return n * lua_test_factorial(n - 1)
	end
end

-- Check if value is nil
function lua_test_is_nil(val)
	return val == nil
end

-- Mixed type array handling
function lua_test_process_mixed(arr)
	local result = {}
	for i = 1, #arr do
		result[i] = tostring(arr[i])
	end
	return result
end
