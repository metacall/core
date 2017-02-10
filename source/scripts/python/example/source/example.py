#!/usr/bin/python3.4

def dont_load_this_function(left, right):
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def multiply(left: int, right: int) -> int:
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def divide(left: float, right: float) -> float:
	if right != 0.0:
		result = left / right;
		print(left, ' / ', right, ' = ', result);
	else:
		print('Invalid right operand: ', right);
	return result;

def sum(left: int, right: int) -> int:
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;

def hello():
	print('Hello World from Python!!');
	return;

def strcat(left: str, right: str) -> str:
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;
