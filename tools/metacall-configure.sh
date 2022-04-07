#!/usr/bin/env bash

#
#	MetaCall Build Bash Script by Parra Studios
#	Build and install bash script utility for MetaCall.
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

RUN_AS_ROOT=0
SUDO_CMD=sudo
BUILD_TYPE=Release
BUILD_PYTHON=0
BUILD_RUBY=0
BUILD_NETCORE=0
BUILD_NETCORE2=0
BUILD_NETCORE5=0
BUILD_V8=0
BUILD_NODEJS=0
BUILD_TYPESCRIPT=0
BUILD_FILE=0
BUILD_RPC=0
BUILD_WASM=0
BUILD_JAVA=0
BUILD_C=0
BUILD_SCRIPTS=0
BUILD_EXAMPLES=0
BUILD_TESTS=0
BUILD_BENCHMARKS=0
BUILD_PORTS=0
BUILD_COVERAGE=0
BUILD_SANITIZER=0

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
		if [ "$option" = 'netcore2' ]; then
			echo "Build with netcore 2 support"
			BUILD_NETCORE2=1
		fi
		if [ "$option" = 'netcore5' ]; then
			echo "Build with netcore 5 support"
			BUILD_NETCORE5=1
		fi
		if [ "$option" = 'v8' ]; then
			echo "Build with v8 support"
			BUILD_V8=1
		fi
		if [ "$option" = 'nodejs' ]; then
			echo "Build with nodejs support"
			BUILD_NODEJS=1
		fi
		if [ "$option" = 'typescript' ]; then
			echo "Build with typescript support"
			BUILD_TYPESCRIPT=1
		fi
		if [ "$option" = 'file' ]; then
			echo "Build with file support"
			BUILD_FILE=1
		fi
		if [ "$option" = 'rpc' ]; then
			echo "Build with rpc support"
			BUILD_RPC=1
		fi
		if [ "$option" = 'wasm' ]; then
			echo "Build with wasm support"
			BUILD_WASM=1
		fi
		if [ "$option" = 'java' ]; then
			echo "Build with java support"
			BUILD_JAVA=1
		fi
		if [ "$option" = 'c' ]; then
			echo "Build with c support"
			BUILD_C=1
		fi
		if [ "$option" = 'scripts' ]; then
			echo "Build all scripts"
			BUILD_SCRIPTS=1
		fi
		if [ "$option" = 'examples' ]; then
			echo "Build all examples"
			BUILD_EXAMPLES=1
		fi
		if [ "$option" = 'tests' ]; then
			echo "Build all tests"
			BUILD_TESTS=1
		fi
		if [ "$option" = 'benchmarks' ]; then
			echo "Build all benchmarks"
			BUILD_BENCHMARKS=1
		fi
		if [ "$option" = 'ports' ]; then
			echo "Build all ports"
			BUILD_PORTS=1
		fi
		if [ "$option" = 'coverage' ]; then
			echo "Build all coverage reports"
			BUILD_COVERAGE=1
		fi
		if [ "$option" = 'sanitizer' ]; then
			echo "Build with sanitizers"
			BUILD_SANITIZER=1
		fi
	done
}

sub_configure() {
	BUILD_STRING="-DOPTION_BUILD_LOG_PRETTY=Off \
			-DOPTION_BUILD_LOADERS=On \
			-DOPTION_BUILD_LOADERS_MOCK=On"

	# Scripts
	if [ $BUILD_SCRIPTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=Off"
	fi

	# Python
	if [ $BUILD_PYTHON = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_PY=On"

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
			-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.10/"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		fi
	fi

	# NetCore 2
	if [ $BUILD_NETCORE2 = 1 ]; then
		BUILD_STRING="$BUILD_STRING \
			-DOPTION_BUILD_LOADERS_CS=On \
			-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/2.2.8/"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		fi
	fi

	# NetCore 5
	if [ $BUILD_NETCORE5 = 1 ]; then
		BUILD_STRING="$BUILD_STRING \
			-DOPTION_BUILD_LOADERS_CS=On \
			-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/5.0.15/"

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

	# TypeScript
	if [ $BUILD_TYPESCRIPT = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_TS=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_TS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_TS=On"
		fi
	fi

	# File
	if [ $BUILD_FILE = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_FILE=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_FILE=On"
		fi
	fi

	# RPC
	if [ $BUILD_RPC = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_RPC=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RPC=On"
		fi
	fi

	# WebAssembly
	if [ $BUILD_WASM = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_WASM=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_WASM=On"
		fi
	fi

	# Java
	if [ $BUILD_JAVA = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_JAVA=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_JAVA=On"
		fi
	fi

	# C
	if [ $BUILD_C = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_C=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_C=On"
		fi
	fi

	# Examples
	if [ $BUILD_EXAMPLES = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_EXAMPLES=Off"
	fi

	# Tests
	if [ $BUILD_TESTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_TESTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_TESTS=Off"
	fi

	# Benchmarks
	if [ $BUILD_BENCHMARKS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_BENCHMARKS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_BENCHMARKS=Off"
	fi

	# Ports
	if [ $BUILD_PORTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS=Off"
	fi

	# Coverage
	if [ $BUILD_COVERAGE = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_COVERAGE=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_COVERAGE=Off"
	fi

	# Sanitizer
	if [ $BUILD_SANITIZER = 1 ]; then
		# Disable backtrace module when sanitizer is enabled
		# in order to let the sanitizer catch the segmentation faults
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SANITIZER=On -DOPTION_BUILD_BACKTRACE=Off"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SANITIZER=Off"
	fi

	# Build type
	BUILD_STRING="$BUILD_STRING -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

	# Execute CMake
	cmake -Wno-dev -DOPTION_GIT_HOOKS=Off $BUILD_STRING ..
}

sub_help() {
	echo "Usage: `basename "$0"` list of options"
	echo "Options:"
	echo "	root: build being run by root"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	python: build with python support"
	echo "	ruby: build with ruby support"
	echo "	netcore: build with netcore support"
	echo "	netcore2: build with netcore 2 support"
	echo "	netcore5: build with netcore 5 support"
	echo "	v8: build with v8 support"
	echo "	nodejs: build with nodejs support"
	echo "	typescript: build with typescript support"
	echo "	file: build with file support"
	echo "	rpc: build with rpc support"
	echo "	wasm: build with wasm support"
	echo "	java: build with java support"
	echo "	c: build with c support"
	echo "	scripts: build all scripts"
	echo "	examples: build all examples"
	echo "	tests: build and run all tests"
	echo "	benchmarks: build and run all benchmarks"
	echo "	install: install all libraries"
	echo "	static: build as static libraries"
	echo "	dynamic: build as dynamic libraries"
	echo "	ports: build all ports"
	echo "	coverage: build all coverage reports"
	echo "	sanitizer: build with address, memory, thread... sanitizers"
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
