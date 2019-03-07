#!/usr/bin/python3.5

from os import path

# Load template html
basepath = path.dirname(path.abspath(__file__))

try:
	with open(path.join(basepath, 'template.html'), 'r') as f:
		template = f.read()
except IOError:
	template = '<html><head></head><body>Error</body></html>'


def landing():
	return template;
