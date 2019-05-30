"""A setuptools based setup module.

See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/pypa/sampleproject
"""

# To use a consistent encoding
from codecs import open
from os import path
# Always prefer setuptools over distutils
from setuptools import setup, find_packages

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
	long_description = f.read()

setup(
	name='metacall',

	# Versions should comply with PEP440.  For a discussion on single-sourcing
	# the version across setup.py and the project code, see
	# https://packaging.python.org/en/latest/single_source_version.html
	version='0.1.9',

	description='A library for providing inter-language foreign function interface calls',
	long_description=long_description,
	long_description_content_type='text/x-rst',

	# The project's main homepage
	url='https://github.com/metacall/core',

	# Author details
	author='Vicente Eduardo Ferrer Garcia',
	author_email='vic798@gmail.com',

	# License
	license='Apache License 2.0',

	# See https://pypi.python.org/pypi?%3Aaction=list_classifiers
	classifiers=[
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
	keywords='metacall python port ffi polyglot faas serverless',

	# Required packages
	packages=find_packages(exclude=['contrib', 'docs', 'tests']),

	# Modules
	py_modules=["metacall"],

	# List run-time dependencies here.  These will be installed by pip when
	# your project is installed. For an analysis of "install_requires" vs pip's
	# requirements files see:
	# https://packaging.python.org/en/latest/requirements.html
	install_requires=['peppercorn', 'requests'],

	# List additional groups of dependencies here (e.g. development
	# dependencies). You can install these using the following syntax,
	# for example:
	# $ pip install -e .[dev,test]
	extras_require={
		'dev': ['check-manifest'],
		'test': ['coverage'],
	},

	# If there are data files included in your packages that need to be
	# installed, specify them here.  If using Python 2.6 or less, then these
	# have to be included in MANIFEST.in as well.
	#package_data={
	#    'sample': ['package_data.dat'],
	#},

	# Although 'package_data' is the preferred approach, in some case you may
	# need to place data files outside of your packages. See:
	# http://docs.python.org/3.4/distutils/setupscript.html#installing-additional-files # noqa
	# In this case, 'data_file' will be installed into '<sys.prefix>/my_data'
	#data_files=[('my_data', ['data/data_file'])],

	# To provide executable scripts, use entry points in preference to the
	# "scripts" keyword. Entry points provide cross-platform support and allow
	# pip to create the appropriate form of executable for the target platform.
	entry_points={
		'console_scripts': [
			'metacall-install=helper:install',
			'metacall-uninstall=helper:uninstall_prompt',
			'metacall-update=helper:update'
		],
	},
)
