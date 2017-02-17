#require_relative 'rb_portd'

@dic={}

def cache_initialize()
	@dic={}

	puts 'ruby: initialize'

	#Rb_portd.metacall_load_from_file('py', [ 'frontend.py' ])
	#result = Rb_portd.metacall('frontend_initialize', 9000)
	#puts result

	#result = Rb_portd.metacall('complex_algorithm', 'hello world')
	#puts result

end

def cache_has_key(key: String)

	puts 'ruby: has key'

	return @dic.has_key? key

end

def cache_set(key: String, value: String)

	puts 'ruby: set value'

	@dic[key] = value

end

def cache_get(key: String)

	puts 'ruby: get value'

	@dic[key]

end
