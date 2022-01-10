#!/usr/bin/env python

"""MetaCall Examples - Auth Function Mesh.

[MetaCall](https://metacall.io)
[MetaCall Examples](https://github.com/metacall/examples)

This modules demonstrates a basic example of a python backend that executes
a call to another backend written in JavaScript (NodeJS).

""" 

from metacall import metacall_load_from_file, metacall

metacall_load_from_file('node', ['auth-function-mesh/auth/auth.js'])

def encrypt(text: float) -> float:
	return metacall('sign', text)

def decrypt(token: float) -> float:
	return metacall('verify', token)
