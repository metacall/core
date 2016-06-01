#!/usr/bin/python3.4

def multiply(left: int, right: int):
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def divide(left: float, right: float):
	if right != 0.0:
		result = left / right;
		print(left, ' / ', right, ' = ', result);
	else:
		print('Invalid right operand: ', right);
	return result;

def sum(left: int, right: int):
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;

def hello():
	print('Hello World from Python!!');
	return;
