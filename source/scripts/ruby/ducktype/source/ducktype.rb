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
