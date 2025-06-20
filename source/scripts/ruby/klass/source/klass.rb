#!/usr/bin/ruby

class MyClass 
	
	@@class_hierarchy_var = 555
	@yeet = 3

	def initialize(param1, param2)  
	  # Instance variables  
	  @param1 = param1  
	  @param2 = param2

	  @param3 = 777
	end

	def return_bye(value)
		result = 'Bye ' + value + '!'
		puts(result)
		return result
	end

	def many_parameters(a, b, c, d, e, f, g, h)
		puts(a, b, c, d, e, f, g, h)
	end
	
	def self.static_hello(value)
		result = 'Hello static ' + value + '!'
		puts(result)
		return result
	end

end  

def return_object_function()
	return MyClass.new("param1", 2)
end

def return_class_function()
	return MyClass
end

#p return_class_function()::CLASS_CONSTANT
