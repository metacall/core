#!/usr/bin/env python3

def nice_dict():
	return { 'hello': 'world', 'asd': 123, 'efg': 3.4 }

def non_supported_dict():
	# The string key is supported but the integer is not, is this acceptable?
	return { 3244: 'world', 'asd': 123 }

def with_love_for_pragma_devs(d):
	return { 'new': d['old'], 'whatever': d['whatever'] }
