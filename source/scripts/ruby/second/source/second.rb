#!/usr/bin/ruby

def get_second(first: Fixnum, second: Fixnum)
	puts('Second value is', second)
	return second
end

def get_second_untyped(first, second)
	puts('Second untyped value is', second)
	return second
end
