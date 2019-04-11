#!/usr/bin/python3.5

def s_dont_load_this_function(left, right):
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def s_multiply(left: int, right: int) -> int:
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def s_divide(left: float, right: float) -> float:
	if right != 0.0:
		result = left / right;
		print(left, ' / ', right, ' = ', result);
	else:
		print('Invalid right operand: ', right);
	return result;

def s_sum(left: int, right: int) -> int:
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;

def s_hello():
	print('Hello World from Python!!');
	return;

def s_strcat(left: str, right: str) -> str:
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;
