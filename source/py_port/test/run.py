#!/usr/bin/python3.4

import os

os.environ['LOADER_LIBRARY_PATH'] = '/media/sf_runscape_boy/metacall/build/'
os.environ['LOADER_SCRIPT_PATH'] = '/media/sf_runscape_boy/metacall/build/scripts/'

import metacall

if metacall.metacall_initialize() == 0:
	print("Valid initialization")
else:
	print("Error initialization")

# MetaCall (Python)
metacall.metacall("hello")

metacall.metacall("multiply", 5, 7)

# MetaCall (Ruby)
metacall.metacall("say_null")

metacall.metacall("say_multiply", 3, 4)

metacall.metacall("say_hello", "helloooo")
