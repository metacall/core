#!/usr/bin/env python3

import sys
from os import path, environ

# TODO: Refactor MetaCall py port dependency into requirements.txt

sys.path.append(environ['PORT_LIBRARY_PATH'])

try:
	from _py_portd import metacall_load_from_file, metacall
except ImportError:
	from _py_port import metacall_load_from_file, metacall

print('[DEBUG] PyPort: ' + str(metacall))

# Load scripts
metacall_load_from_file('node', ['auth/auth.js'])

metacall_load_from_file('cs', ['ads/Ads.cs', 'ads/AdsImpression.cs', 'ads/AdsProviderGambling.cs', 'ads/AdsProviderClothes.cs', 'ads/IAdsProvider.cs'])

# Load index html
basepath = path.dirname(path.abspath(__file__))

try:
	with open(path.join(basepath, 'template.html'), 'r') as f:
		template = f.read()
except IOError:
	template = '<html><head></head><body>Error</body></html>'

# Functions
def index():
	return template

def login(text):
	return metacall('sign', text)

def time(token):
	return metacall('verify', token)

def ads():
	return metacall('Ads')
