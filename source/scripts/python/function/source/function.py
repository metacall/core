#!/usr/bin/env python3

import sys

def function_cb(cb):
	print('Executing:', cb)
	return cb()

def function_return():
	return lambda: 4

def function_with_args(cb, left, right):
	print('Executing:', cb, '(', left, ',', right, ')')
	return cb(left, right)

def function_print_and_return(x):
	print('Executing lambda with argument:', x)
	return x

def function_ret_lambda(y):
	print('Returning lambda with captured arg:', y)
	return lambda x: function_print_and_return(x) * y

def function_currying(y):
	return lambda x: lambda z: x * z * y

def function_currying_more(y):
	return lambda x: lambda z: lambda w: lambda n: x * z * w * n * y

def function_pass():
	pass

class MyClass:
	def f(self):
		return 'hello world'

def function_myclass_new_class():
	return MyClass()

def function_myclass_method(klass):
	print('Executing class method with instance passed as opaque pointer:', klass.f())
	return klass.f()

def function_myclass_cb(cb):
	return cb(MyClass())

def function_sum(value, f):
	return 0 if value <= 0 else value + f(value - 1, function_sum)

def function_chain_impl(x, n):
	print('------------------ py chain', n)
	print('------------------ py chain pre x call', x)
	sys.stdout.flush()
	result = x(x)(n)
	print('------------------ py chain post x call', x)
	sys.stdout.flush()
	return result

def function_chain(x):
	return lambda n: function_chain_impl(x, n)

def py_function_factorial_impl(x, n):
	print('------------------ py factorial', n)
	sys.stdout.flush()
	if n == 0:
		print('------------------ py factorial case base')
		sys.stdout.flush()
		return 1
	else:
		print('------------------ py factorial pre x() call', x)
		sys.stdout.flush()
		result = n * x(x)(n - 1)
		print('------------------ py factorial post x() call', x)
		sys.stdout.flush()
		return result

def py_function_factorial(x):
	return lambda n: py_function_factorial_impl(x, n)

def function_factorial(x):
	return lambda n: 1 if n <= 0 else n * x(x)(n - 1)

def func_equality_impl():
	return 3

def func_equality():
	return func_equality_impl

def func_equality_compare(f1, f2):
	return f1 == f2
