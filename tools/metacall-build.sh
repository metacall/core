#!/usr/bin/env sh

#
#	MetaCall Build Shell Script by Parra Studios
#	Build and install shell script utility for MetaCall.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#

set -euxo

BUILD_TYPE=Release
BUILD_TESTS=0
BUILD_BENCHMARKS=0
BUILD_COVERAGE=0
BUILD_INSTALL=0

# Check out for sudo
if [ "`id -u`" = '0' ]; then
	SUDO_CMD=""
else
	SUDO_CMD=sudo
fi

sub_options() {
	for option in "$@"
	do
		if [ "$option" = 'debug' ]; then
			echo "Build all scripts in debug mode"
			BUILD_TYPE=Debug
		fi
		if [ "$option" = 'release' ]; then
			echo "Build all scripts in release mode"
			BUILD_TYPE=Release
		fi
		if [ "$option" = 'relwithdebinfo' ]; then
			echo "Build all scripts in release mode with debug symbols"
			BUILD_TYPE=RelWithDebInfo
		fi
		if [ "$option" = 'tests' ]; then
			echo "Build and run all tests"
			BUILD_TESTS=1
		fi
		if [ "$option" = 'benchmarks' ]; then
			echo "Build and run all benchmarks"
			BUILD_BENCHMARKS=1
		fi
		if [ "$option" = 'coverage' ]; then
			echo "Build coverage reports"
			BUILD_COVERAGE=1
		fi
		if [ "$option" = 'install' ]; then
			echo "Install all libraries"
			BUILD_INSTALL=1
		fi
	done
}

sub_build() {

	# Build the project
	make -k -j$(getconf _NPROCESSORS_ONLN)

	# Tests (coverage needs to run the tests)
	if [ $BUILD_TESTS = 1 ] || [ $BUILD_BENCHMARKS=1 ] || [ $BUILD_COVERAGE = 1 ]; then
		ctest -j$(getconf _NPROCESSORS_ONLN) --timeout 5400 --output-on-failure --test-output-size-failed 3221000000 -C $BUILD_TYPE
	fi

	# Coverage
	if [ $BUILD_COVERAGE = 1 ]; then
		# TODO: Remove -k, solve coverage issues
		make -k gcov
		make -k lcov
		make -k lcov-genhtml
	fi

	# Install
	if [ $BUILD_INSTALL = 1 ]; then
		if [ "$SUDO_CMD" = "" ]; then
			make install
		else
			# Needed for rustup in order to install rust loader properly
			$SUDO_CMD HOME="$HOME" make install
		fi
	fi
}

sub_help() {
	echo "Usage: `basename "$0"` list of options"
	echo "Options:"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	tests: build and run all tests"
	echo "	coverage: build coverage reports"
	echo "	install: install all libraries"
	echo ""
}

case "$#" in
	0)
		sub_help
		;;
	*)
		sub_options $@
		sub_build
		;;
esac
