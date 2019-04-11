#!/usr/bin/ruby

def say_hello(value)
	result = 'Hello ' + value + '!'
	puts(result)
	return result
end

def say_multiply(left, right)
	result = left * right
	puts('Multiply', result, '!')
	return result
end

def say_null()
	puts('Helloooo from null method!')
end

def get_second(first, second)
	puts('Second value is', second)
	return second
end

def mixed(a, b, c: Fixnum, d: Fixnum)
	puts('Mixed arguments:', a, b, c, d)
	return a + b + c + d
end

def map_style(a: Fixnum, b: Fixnum)
	puts('Old style arguments:', a, b)
	return a + b
end
