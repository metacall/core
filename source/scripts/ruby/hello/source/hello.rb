#!/usr/bin/ruby

def say_hello(value: String)
	result = 'Hello ' + value + '!'
	puts(result)
	return result
end

def say_multiply(left: Fixnum, right: Fixnum)
	result = left * right
	puts('Multiply', result, '!')
	return result
end

def say_null()
	puts('Helloooo from null method!')
end

def backwardsPrime(start, stop)
	# https://github.com/miarhost/Solved_tasks/blob/master/backwards_prime.rb
	ar = []
	ar1 =[]
	range=(start..stop).to_a
	dividers=(2..5000).to_a  #dividers=(2..(int2-1)).to_a has too much complexity for big numbers

	for j in range
		ar= dividers.map{|i| j % i if j != i}
		if ar.include?(0) == false
			ar1 << j
		end
	end

	rp=1
	ar2 = []
	back_primes = []
	ar1.each do |i|
		rp=i.to_s.reverse.to_i
		ar2= dividers.map{|div| rp % div if rp != div}
		if ar2.include?(0) == false && rp != i
			back_primes << i
		end
	end

	return back_primes  #p back_primes
end
