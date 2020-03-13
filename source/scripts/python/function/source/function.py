#!/usr/bin/env python3

def function(cb):
	print('Executing:', cb);
	return cb();

def function_with_args(cb, left, right):
	print('Executing:', cb, '(', left, ',', right, ')');
	return cb(left, right);

def print_and_return(x):
	print('Executing lambda with argument:', x);
	return x;

def function_ret_lambda(y):
	print('Returning lambda with captured arg:', y);
	return lambda x: print_and_return(x) * y
