#!/usr/bin/ruby

require 'test/unit'
require File.expand_path("../package/lib/metacall.rb")

class RbPortTest < Test::Unit::TestCase

	# MetaCall (Python from memory)
	def test_python_memory
		script = '#!/usr/bin/env python3\n' \
			'def inline_multiply_mem(left: int, right: int) -> int:\n' \
			'	return left * right;\n'
			'def inline_hello(left: int, right: int) -> int:\n' \
			'	print(\'Helloo\', left, \' \', right);\n'
			'	return;\n'

		assert_equal(0, MetaCall.metacall_load_from_memory('py', script))

		# TODO
		# assert_equal(4, MetaCall::metacall('inline_multiply_mem', 2, 2))

		assert_equal(nil, MetaCall::metacall('inline_hello', 10, 20))
	end

	# MetaCall (Python)
	def test_python
		assert_equal(0, MetaCall::metacall_load_from_file('py', ['example.py']))

		assert_equal(nil, MetaCall::metacall('hello'))

		# TODO
		# assert_equal(35, MetaCall::metacall('multiply', 5, 7))
	end

	# MetaCall (Ruby)
	def test_ruby
		assert_equal(0, MetaCall::metacall_load_from_file('rb', ['hello.rb']))

		assert_equal(nil, MetaCall::metacall('say_null'))

		# TODO
		# assert_equal(12, MetaCall::metacall('say_multiply', 3, 4))

		# TODO
		# assert_equal('Hello world!', MetaCall::metacall('say_hello', 'world'))
	end

end
