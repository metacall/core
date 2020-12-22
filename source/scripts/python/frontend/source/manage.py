#!/usr/bin/env python3

import os
import sys

def frontend_execute(args):
	os.environ.setdefault("DJANGO_SETTINGS_MODULE", "frontend.settings")
	try:
		from django.core.management import execute_from_command_line
	except ImportError:
		# The above import may fail for some other reason. Ensure that the
		# issue is really that Django is missing to avoid masking other
		# exceptions on Python 2.
		try:
			import django
		except ImportError:
			raise ImportError(
				"Couldn't import Django. Are you sure it's installed and "
				"available on your PYTHONPATH environment variable? Did you "
				"forget to activate a virtual environment?"
			)
		raise
	sys.argv = args
	execute_from_command_line(sys.argv)

def frontend_initialize(port: int) -> int:
	frontend_execute([os.path.abspath(__file__), 'runserver', str(port), '--noreload'])
	return 0

if __name__ == "__main__":
	frontend_execute(sys.argv)
