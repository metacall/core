#!/bin/bash

#
# MetaCall Build Bash Script by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Build and install bash script utility for MetaCall.
#

RUN_AS_ROOT=0
SUDO_CMD=sudo
BUILD_TYPE=Release
BUILD_PYTHON=0
BUILD_RUBY=0
BUILD_NETCORE=0
BUILD_V8=0
BUILD_NODEJS=0
BUILD_PORTS=0
BUILD_SCRIPTS=0
BUILD_EXAMPLES=0

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
		if [ "$option" = 'python' ]; then
			echo "Build with python support"
			BUILD_PYTHON=1
		fi
		if [ "$option" = 'ruby' ]; then
			echo "Build with ruby support"
			BUILD_RUBY=1
		fi
		if [ "$option" = 'netcore' ]; then
			echo "Build with netcore support"
			BUILD_NETCORE=1
		fi
		if [ "$option" = 'v8' ]; then
			echo "Build with v8 support"
			BUILD_V8=1
		fi
		if [ "$option" = 'nodejs' ]; then
			echo "Build with nodejs support"
			BUILD_NODEJS=1
		fi
		if [ "$option" = 'ports' ]; then
			echo "Build all ports"
			BUILD_PORTS=1
		fi
		if [ "$option" = 'scripts' ]; then
			echo "Build all scripts"
			BUILD_SCRIPTS=1
		fi
		if [ "$option" = 'examples' ]; then
			echo "Build all examples"
			BUILD_EXAMPLES=1
		fi
	done
}

sub_configure() {
	BUILD_STRING="-DBUILD_DISTRIBUTABLE_LIBS=Off \
			-DOPTION_BUILD_LOADERS=On \
			-DOPTION_BUILD_LOADERS_MOCK=On"

	# Ports
	if [ $BUILD_PORTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS=Off"
	fi

	# Scripts
	if [ $BUILD_SCRIPTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=Off"
	fi

	# Python
	if [ $BUILD_PYTHON = 1 ]; then
		BUILD_STRING="$BUILD_STRING \
			-DPYTHON_EXECUTABLE=/usr/bin/python3.5 \
			-DOPTION_BUILD_LOADERS_PY=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_PY=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_PY=On"
		fi
	fi

	# Ruby
	if [ $BUILD_RUBY = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_RB=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RB=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_RB=On"
		fi
	fi

	# NetCore
	if [ $BUILD_NETCORE = 1 ]; then
		BUILD_STRING="$BUILD_STRING \
			-DOPTION_BUILD_LOADERS_CS=On \
			-DOPTION_BUILD_LOADERS_CS_IMPL=On \
			-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.0/"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		fi
	fi

	# V8
	if [ $BUILD_V8 = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_JS=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_JS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_JS=On"
		fi
	fi

	# NodeJS
	if [ $BUILD_NODEJS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_NODE=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_NODE=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_NODE=On"
		fi
	fi

	# Examples
	if [ $BUILD_EXAMPLES = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=Off"
	fi

	# Build type
	BUILD_STRING="$BUILD_STRING -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

	# Execute CMake
	cmake $BUILD_STRING ..
}

sub_help() {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
	echo "	root: build being run by root"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	python: build with python support"
	echo "	ruby: build with ruby support"
	echo "	netcore: build with netcore support"
	echo "	v8: build with v8 support"
	echo "	nodejs: build with nodejs support"
	echo "	ports: build all ports"
	echo "	scripts: build all scripts"
	echo "	examples: build all examples"
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
		sub_configure
        ;;
esac
