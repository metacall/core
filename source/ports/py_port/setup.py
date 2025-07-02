#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

# To use a consistent encoding
from codecs import open
import os
import sys
import re

# Always prefer setuptools over distutils
from setuptools import setup, find_packages

current_path = os.path.abspath(os.path.dirname(__file__))

# Get the long description from the README file
with open(os.path.join(current_path, 'README.rst'), encoding='utf-8') as f:
	long_description = f.read()

# Define set up options
options = {
	'name': 'metacall',

	# Versions should comply with PEP440.  For a discussion on single-sourcing
	# the version across setup.py and the project code, see
	# https://packaging.python.org/en/latest/single_source_version.html
	'version': '0.5.2',

	'description': 'A library for providing inter-language foreign function interface calls',
	'long_description': long_description,
	'long_description_content_type': 'text/x-rst',

	# The project's main homepage
	'url': 'https://github.com/metacall/core',

	# Author details
	'author': 'Vicente Eduardo Ferrer Garcia',
	'author_email': 'vic798@gmail.com',

	# License
	'license': 'Apache License 2.0',
	'license_files': 'LICENSE.txt',

	# See https://pypi.python.org/pypi?%3Aaction=list_classifiers
	'classifiers': [
		# Project Status
		#   3 - Alpha
		#   4 - Beta
		#   5 - Production/Stable
		'Development Status :: 4 - Beta',

		# Audience
		'Intended Audience :: Developers',
		'Topic :: Software Development :: Interpreters',

		# Python versions support
		#'Programming Language :: Python :: 2',
		#'Programming Language :: Python :: 2.6',
		#'Programming Language :: Python :: 2.7',
		'Programming Language :: Python :: 3',
		'Programming Language :: Python :: 3.3',
		'Programming Language :: Python :: 3.4',
		'Programming Language :: Python :: 3.5',
		'Programming Language :: Python :: 3.6',
		'Programming Language :: Python :: 3.7',
		'Programming Language :: Python :: 3.8',
		'Programming Language :: Python :: 3.9',
		'Programming Language :: Python :: 3.10',
		'Programming Language :: Python :: 3.11',
		'Programming Language :: Python :: 3.12',
		'Programming Language :: Python :: 3.13',
	],

	# Keywords
	'keywords': 'metacall python port ffi polyglot faas serverless',

	# Modules
	'py_modules': ['metacall'],

	# List additional groups of dependencies here (e.g. development
	# dependencies). You can install these using the following syntax,
	# for example:
	# $ pip install -e .[dev,test]
	'extras_require': {
		'dev': ['check-manifest'],
		'test': ['coverage'],
	},

	# If there are data files included in your packages that need to be
	# installed, specify them here.  If using Python 2.6 or less, then these
	# have to be included in MANIFEST.in as well.
	#package_data: {
	#    'sample': ['package_data.dat'],
	#},

	# Although 'package_data' is the preferred approach, in some case you may
	# need to place data files outside of your packages. See:
	# http://docs.python.org/3.4/distutils/setupscript.html#installing-additional-files # noqa
	# In this case, 'data_file' will be installed into '<sys.prefix>/my_data'
	#data_files: [('my_data', ['data/data_file'])],
}

# Exclude base packages
exclude_packages = ['contrib', 'docs', 'test', 'test.py' 'CMakeLists.txt', '.gitignore', 'upload.sh']

# Define required packages
options['packages'] = find_packages(exclude=exclude_packages)

# Execute the setup
setup(**options)
