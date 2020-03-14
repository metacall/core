#!/usr/bin/env python3

def function(cb):
	print('Executing:', cb);
	return cb();

def function_return():
	return lambda: 4

def function_with_args(cb, left, right):
	print('Executing:', cb, '(', left, ',', right, ')');
	return cb(left, right);

def function_print_and_return(x):
	print('Executing lambda with argument:', x);
	return x;

def function_ret_lambda(y):
	print('Returning lambda with captured arg:', y);
	return lambda x: function_print_and_return(x) * y

def function_currying(y):
	return lambda x: lambda z: x * z * y

def function_currying_more(y):
	return lambda x: lambda z: lambda w: lambda n: x * z * w * n * y

def function_factorial_compose(x):
	return lambda n: x(x)(n)

def function_factorial(x):
	return lambda n: 1 if n == 0 else n * x(x)(n - 1)
