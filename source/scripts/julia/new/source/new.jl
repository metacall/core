using LinearAlgebra

function plus(a, b)
	return a + b
end

minus(a, b) = a - b

# TODO: Overloading is not implemented in the core yet, this should be reviewed or
# implemented ad-hoc in Julia loader when resolving the calls in the invoke
function plus(a)
	return a+1
end

function mul(a,b)
	return a*b
end
