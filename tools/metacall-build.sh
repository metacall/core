#!/bin/bash

#
#	MetaCall Build Bash Script by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Build and install bash script utility for MetaCall.
#

RUN_AS_ROOT=0
SUDO_CMD=sudo
BUILD_TYPE=Release
BUILD_DISTRIBUTABLE=0
BUILD_STATIC=0
BUILD_DYNAMIC=0
BUILD_TESTS=0
BUILD_INSTALL=0

sub_options() {
	for option in "$@"
	do
		if [ "$option" = 'root' ]; then
			echo "Running build script as root"
			RUN_AS_ROOT=1
			SUDO_CMD=""
		fi
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
		if [ "$option" = 'distributable' ]; then
			echo "Build distributable libraries"
			BUILD_DISTRIBUTABLE=1
		fi
		if [ "$option" = 'static' ]; then
			echo "Build static libraries"
			BUILD_STATIC=1
		fi
		if [ "$option" = 'dynamic' ]; then
			echo "Build dynamic libraries"
			BUILD_DYNAMIC=1
		fi
		if [ "$option" = 'tests' ]; then
			echo "Build and run all tests"
			BUILD_TESTS=1
		fi
		if [ "$option" = 'install' ]; then
			echo "Install all libraries"
			BUILD_INSTALL=1
		fi
	done
}

sub_build() {
	# Execute CMake without distributable
	cmake -DBUILD_DISTRIBUTABLE_LIBS=Off ..

	# Make without distributable
	make -j$(getconf _NPROCESSORS_ONLN)

	# Build with distributable
	if [ $BUILD_DISTRIBUTABLE = 1 ]; then
		cmake -DBUILD_DISTRIBUTABLE_LIBS=On ..
		make -j$(getconf _NPROCESSORS_ONLN)
	fi

	# Build without scripts on release
	if [ $BUILD_INSTALL = 1 ]; then
		if [ "$BUILD_TYPE" = 'Release' ] || [ "$BUILD_TYPE" = 'RelWithDebInfo' ]; then
			cmake -DOPTION_BUILD_SCRIPTS=Off ..
		fi
	fi

	# Build as dynamic libraries
	if [ $BUILD_DYNAMIC = 1 ]; then
		cmake -DBUILD_SHARED_LIBS=On ..
		make -j$(getconf _NPROCESSORS_ONLN)

		if [ $BUILD_INSTALL = 1 ]; then
			$SUDO_CMD make install
		fi
	fi

	# Build as static libraries
	if [ $BUILD_STATIC = 1 ]; then
		cmake -DBUILD_SHARED_LIBS=Off ..
		make -j$(getconf _NPROCESSORS_ONLN)

		if [ $BUILD_INSTALL = 1 ]; then
			$SUDO_CMD make install
		fi
	fi

	# Install
	if [ $BUILD_INSTALL = 1 ] && [ $BUILD_STATIC = 0 ] && [ $BUILD_DYNAMIC = 0 ]; then
		$SUDO_CMD make install
	fi

	# Tests
	if [ $BUILD_TESTS = 1 ]; then
		ctest -VV -C $BUILD_TYPE
	fi
}

sub_help() {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
	echo "	root: build being run by root"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	distributable: build distributable libraries"
	echo "	tests: build and run all tests"
	echo "	install: install all libraries"
	echo "	static: build as static libraries"
	echo "	dynamic: build as dynamic libraries"
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
