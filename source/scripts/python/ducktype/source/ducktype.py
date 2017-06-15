#!/usr/bin/python3.4

def multiply(left, right):
	result = left * right;
	print(left, ' * ', right, ' = ', result);
	return result;

def divide(left, right):
	if right != 0.0:
		result = left / right;
		print(left, ' / ', right, ' = ', result);
	else:
		print('Invalid right operand: ', right);
	return result;

def sum(left, right):
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;

def hello():
	print('Hello World from Python!!');
	return;

def strcat(left, right):
	result = left + right;
	print(left, ' + ', right, ' = ', result);
	return result;
