#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	'version': '0.1.18',

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

		# License
		'License :: OSI Approved :: Apache Software License',

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
exclude_packages =  ['contrib', 'docs', 'tests']

# Detect if metacall port is already installed
port_installed = False

# Append environment variable or default install path when building manually (TODO: Cross-platform paths)
sys.path.append(os.environ.get('PORT_LIBRARY_PATH', os.path.join(os.path.sep, 'usr', 'local', 'lib')));

# Find is MetaCall is installed as a distributable tarball (TODO: Cross-platform paths)
rootdir = os.path.join(os.path.sep, 'gnu', 'store')
regex = re.compile('.*-metacall-.*')

for root, dirs, _ in os.walk(rootdir):
	for folder in dirs:
		if regex.match(folder) and not folder.endswith('R'):
			sys.path.append(os.path.join(rootdir, folder, 'lib'))

# Find if module is installed
if sys.version_info[0] < 3:
	# Python 2.7
	import imp
	try:
		imp.find_module('_py_port')
		port_installed = True
	except ImportError:
		try:
			imp.find_module('_py_portd')
			port_installed = True
		except ImportError:
			pass
elif sys.version_info[0] >= 3 and sys.version_info[1] <= 3:
	# Python <= 3.3
	import importlib
	py_port = importlib.find_loader('_py_port')
	port_installed = py_port is not None
	if port_installed == False:
		py_port = importlib.find_loader('_py_portd')
		port_installed = py_port is not None
elif sys.version_info[0] >= 3 and sys.version_info[1] > 3:
	# Python >= 3.4
	import importlib
	py_port = importlib.util.find_spec("_py_port")
	port_installed = py_port is not None
	if port_installed == False:
		py_port = importlib.util.find_spec("_py_portd")
		port_installed = py_port is not None

# TODO: This code is very interesting for providing commands to the end user.
#	pip cannot execute arbitrary code as pre/post install hook when the package is being installed.
#	So it is impossible to install the binaries unless we add extra commands after install.
#	At this moment there is a common solution for installing binaries depending on Bash/PowerShell
#	that is OS dependant and not language dependant. By the moment we will use the new way of install
#	instead of the old one, but we keep the ./helper folder in order to provide future support for
#	extra commands, although the main idea is to keep the OS dependant install, this can be useful
#	for updating or doing Python related things. Meanwhile, it will be avoided.
exclude_packages.append('helper')

# if port_installed == True:
# 	# Exclude helper package if port is already installed
# 	exclude_packages.append('helper')
# else:
# 	# List run-time dependencies here.  These will be installed by pip when
# 	# your project is installed. For an analysis of "install_requires" vs pip's
# 	# requirements files see:
# 	# https://packaging.python.org/en/latest/requirements.html
# 	options['install_requires'] = ['peppercorn', 'requests']

# 	# To provide executable scripts, use entry points in preference to the
# 	# "scripts" keyword. Entry points provide cross-platform support and allow
# 	# pip to create the appropriate form of executable for the target platform.
# 	options['entry_points'] = {
# 		'console_scripts': [
# 			'metacall-install=helper:install',
# 			'metacall-uninstall=helper:uninstall_prompt',
# 			'metacall-update=helper:update'
# 		],
# 	}

# Define required packages
options['packages'] = find_packages(exclude=exclude_packages)

# Execute the setup
setup(**options)
