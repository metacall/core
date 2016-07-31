#!/usr/bin/ruby


#def say_hello(value = '')
def say_hello(value: String)
	result = 'Hello ' + value + '!'
	puts(result)
	return result
end

#def say_multiply(left = 0, right = 0)
def say_multiply(left: Fixnum, right: Fixnum)
	result = left * right
	puts('Multiply', result, '!')
	return result
end

def say_null()
	puts('Helloooo from null method!')
end
