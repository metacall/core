--[[
 *	Loader Library by Parra Studios
 *	A plugin for loading lua code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
--]]

-- Arithmetic functions
function multiply(left, right)
	local result = left * right
	print(left .. ' * ' .. right .. ' = ' .. result)
	return result
end

function divide(left, right)
	if right ~= 0.0 then
		local result = left / right
		print(left .. ' / ' .. right .. ' = ' .. result)
		return result
	else
		print('Invalid right operand: ' .. tostring(right))
		return nil
	end
end

function sum(left, right)
	local result = left + right
	print(left .. ' + ' .. right .. ' = ' .. result)
	return result
end

-- String functions
function strcat(left, right)
	local result = left .. right
	print(left .. ' + ' .. right .. ' = ' .. result)
	return result
end

-- Hello world function
function hello()
	print('Hello World from Lua!!')
end

-- Array functions
function return_array()
	return {1, 2, 3}
end

function return_same_array(arr)
	return arr
end

-- Type testing function
function type_test(int_val, double_val, str_val, bool_val)
	print('int: ' .. tostring(int_val))
	print('double: ' .. tostring(double_val))
	print('string: ' .. str_val)
	print('bool: ' .. tostring(bool_val))
	return int_val + double_val, str_val, bool_val
end

-- Boolean function
function bool_not(val)
	return not val
end
