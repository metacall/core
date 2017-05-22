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
BUILD_PORTS=0
BUILD_SCRIPTS=0
BUILD_EXAMPLES=0
BUILD_DISTRIBUTABLE=0
BUILD_TESTS=0
BUILD_INSTALL=0

sub_config() {
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
		if [ "$option" = 'distributable' ]; then
			echo "Build distributable libraries"
			BUILD_DISTRIBUTABLE=1
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
	BUILD_STRING="-DBUILD_DISTRIBUTABLE_LIBS=Off \
			-DOPTION_BUILD_PLUGINS=On \
			-DOPTION_BUILD_PLUGINS_MOCK=On"

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
			-DOPTION_BUILD_PLUGINS_PY=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_PY=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_PY=On"
		fi
	fi

	# Ruby
	if [ $BUILD_RUBY = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PLUGINS_RB=On"

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
			-DOPTION_BUILD_PLUGINS_CS=On \
			-DCORECLR_ROOT_REPOSITORY_PATH=$METACALL_PATH/build/coreclr/ \
			-DOPTION_BUILD_PLUGINS_CS_IMPL=On \
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
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PLUGINS_JS=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_JS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_JS=On"
		fi
	fi

	# Examples
	if [ $BUILD_EXAMPLES = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=Off"
	fi

	# Build Type
	BUILD_STRING="$BUILD_STRING -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

	# Execute CMake without distributable
	cmake $BUILD_STRING ..

	# Make without distributable
	make -j$(getconf _NPROCESSORS_ONLN)

	# Build with distributable
	if [ $BUILD_DISTRIBUTABLE = 1 ]; then
		cmake -DBUILD_DISTRIBUTABLE_LIBS=On ..
		make -j$(getconf _NPROCESSORS_ONLN)
	fi

	# Tests
	if [ $BUILD_TESTS = 1 ]; then
		ctest -VV -C $BUILD_TYPE
	fi

	# Install
	if [ $BUILD_INSTALL = 1 ]; then
		$SUDO_CMD make install
	fi
}

sub_help() {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
	echo "	root: build being run by root"
	echo "	debug | release: build type"
	echo "	python: build with python support"
	echo "	ruby: build with ruby support"
	echo "	netcore: build with netcore support"
	echo "	v8: build with v8 support"
	echo "	ports: build all ports"
	echo "	scripts: build all scripts"
	echo "	examples: build all examples"
	echo "	distributable: build distributable libraries"
	echo "	tests: build and run all tests"
	echo "	install: install all libraries"
	echo "	static | dynamic: build as static or dynamic libraries [TODO]"
	echo ""
}

case "$#" in
    0)
        sub_help
        ;;
    *)
		sub_config $@
		sub_build
        ;;
esac
