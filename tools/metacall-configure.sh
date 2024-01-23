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

ROOT_DIR=$(pwd)
BUILD_TYPE=Release
BUILD_PYTHON=0
BUILD_RUBY=0
BUILD_NETCORE=0
BUILD_NETCORE2=0
BUILD_NETCORE5=0
BUILD_NETCORE7=0
BUILD_V8=0
BUILD_NODEJS=0
BUILD_TYPESCRIPT=0
BUILD_FILE=0
BUILD_RPC=0
BUILD_WASM=0
BUILD_JAVA=0
BUILD_C=0
BUILD_COBOL=0
BUILD_GO=0
BUILD_RUST=0
BUILD_SCRIPTS=0
BUILD_EXAMPLES=0
BUILD_TESTS=0
BUILD_BENCHMARKS=0
BUILD_PORTS=0
BUILD_SANDBOX=0
BUILD_COVERAGE=0
BUILD_ADDRESS_SANITIZER=0
BUILD_THREAD_SANITIZER=0
BUILD_MEMORY_SANITIZER=0

# Operative System detection
case "$(uname -s)" in
	Linux*)		OPERATIVE_SYSTEM=Linux;;
	Darwin*)	OPERATIVE_SYSTEM=Darwin;;
	CYGWIN*)	OPERATIVE_SYSTEM=Cygwin;;
	MINGW*)		OPERATIVE_SYSTEM=MinGW;;
	*)			OPERATIVE_SYSTEM="Unknown"
esac

# Linux Distro detection
if [ -f /etc/os-release ]; then # Either Debian or Ubuntu
	# Cat file | Get the ID field | Remove 'ID=' | Remove leading and trailing spaces
	LINUX_DISTRO=$(cat /etc/os-release | grep "^ID=" | cut -f2- -d= | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
	# Cat file | Get the ID field | Remove 'ID=' | Remove leading and trailing spaces | Remove quotes
	LINUX_VERSION_ID=$(cat /etc/os-release | grep "^VERSION_ID=" | cut -f2- -d= | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr -d '"')
else
	# TODO: Implement more distros or better detection
	LINUX_DISTRO=unknown
	LINUX_VERSION_ID=unknown
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
		if [ "$option" = 'netcore7' ]; then
			echo "Build with netcore 7 support"
			BUILD_NETCORE7=1
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
		if [ "$option" = 'cobol' ]; then
			echo "Build with cobol support"
			BUILD_COBOL=1
		fi
		if [ "$option" = 'go' ]; then
			echo "Build with go support"
			BUILD_GO=1
		fi
		if [ "$option" = 'rust' ]; then
			echo "Build with rust support"
			BUILD_RUST=1
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
		if [ "$option" = 'sandbox' ]; then
			echo "Build with sandboxing support"
			BUILD_SANDBOX=1
		fi
		if [ "$option" = 'coverage' ]; then
			echo "Build all coverage reports"
			BUILD_COVERAGE=1
		fi
		if [ "$option" = 'address-sanitizer' ]; then
			echo "Build with address sanitizer"
			BUILD_ADDRESS_SANITIZER=1
		fi
		if [ "$option" = 'thread-sanitizer' ]; then
			echo "Build with thread sanitizer"
			BUILD_THREAD_SANITIZER=1
		fi
		if [ "$option" = 'memory-sanitizer' ]; then
			echo "Build with memory sanitizer"
			BUILD_MEMORY_SANITIZER=1
		fi
	done
}

sub_find_dotnet_runtime() {
	NETCORE_BASE_PATH=`dotnet --list-runtimes | grep -m 1 "Microsoft.NETCore.App $1"`
	echo "`echo \"$NETCORE_BASE_PATH\" | awk '{ print $3 }' | tail -c +2 | head -c -2`/`echo \"$NETCORE_BASE_PATH\" | awk '{ print $2 }'`/"
}

sub_configure() {
	BUILD_STRING="-DOPTION_BUILD_LOG_PRETTY=Off \
			-DOPTION_BUILD_LOADERS=On \
			-DOPTION_BUILD_LOADERS_MOCK=On"


	# Enable build with musl libc
	if [ "$LINUX_DISTRO" = "alpine" ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_MUSL=On"
	fi

	# Scripts
	if [ $BUILD_SCRIPTS = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS=Off"
	fi

	# Python
	if [ $BUILD_PYTHON = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_PY=On"

		# Patch for Darwin Python headers
		if [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
			BUILD_STRING="$BUILD_STRING -DPython3_INCLUDE_DIR=$(python3 -c "import sysconfig; print(sysconfig.get_path('include'))")"
			# BUILD_STRING="$BUILD_STRING -DPython3_LIBRARY=$(python3 -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))")"
		fi

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
			-DDOTNET_CORE_PATH=`sub_find_dotnet_runtime 1`"

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
			-DDOTNET_CORE_PATH=`sub_find_dotnet_runtime 2`"

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
			-DDOTNET_CORE_PATH=`sub_find_dotnet_runtime 5`"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		fi
	fi

	# NetCore 7
	if [ $BUILD_NETCORE7 = 1 ]; then
		BUILD_STRING="$BUILD_STRING \
			-DOPTION_BUILD_LOADERS_CS=On \
			-DDOTNET_CORE_PATH=`sub_find_dotnet_runtime 7`"

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

		if [ "$LINUX_DISTRO" = "alpine" ]; then
			BUILD_STRING="$BUILD_STRING -DJAVA_HOME=/usr/lib/jvm/java-1.8-openjdk"
		fi

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

	# Cobol
	if [ $BUILD_COBOL = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_COB=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_COB=On"
		fi
	fi

	# Go
	if [ $BUILD_GO = 1 ]; then
		# TODO
		# BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_GO=On"

		# if [ $BUILD_SCRIPTS = 1 ]; then
		# 	BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_GO=On"
		# fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_GO=On"
		fi
	fi

	# Rust
	if [ $BUILD_RUST = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_LOADERS_RS=On"

		if [ $BUILD_SCRIPTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RS=On"
		fi

		if [ $BUILD_PORTS = 1 ]; then
			BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PORTS_RS=On"
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

	# Sandbox
	if [ $BUILD_SANDBOX = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PLUGINS_SANDBOX=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_PLUGINS_SANDBOX=Off"
	fi

	# Coverage
	if [ $BUILD_COVERAGE = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_COVERAGE=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_COVERAGE=Off"
	fi

	# Address Sanitizer
	if [ $BUILD_ADDRESS_SANITIZER = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_ADDRESS_SANITIZER=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_ADDRESS_SANITIZER=Off"
	fi

	# Thread Sanitizer
	if [ $BUILD_THREAD_SANITIZER = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_THREAD_SANITIZER=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_THREAD_SANITIZER=Off"
	fi

	# Memory Sanitizer
	if [ $BUILD_MEMORY_SANITIZER = 1 ]; then
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_MEMORY_SANITIZER=On"
	else
		BUILD_STRING="$BUILD_STRING -DOPTION_BUILD_MEMORY_SANITIZER=Off"
	fi

	# Split cmake config file line by line and add each line to the build string
	CMAKE_CONFIG_FILE="$ROOT_DIR/CMakeConfig.txt"
	if [ -f $CMAKE_CONFIG_FILE ]; then
		while IFS= read -r line
		do
			BUILD_STRING="$BUILD_STRING $line"
		done < "$CMAKE_CONFIG_FILE"
	fi

	# Build type
	BUILD_STRING="$BUILD_STRING -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
	
	# Execute CMake
	cmake -Wno-dev -DOPTION_GIT_HOOKS=Off $BUILD_STRING ..
}

sub_help() {
	echo "Usage: `basename "$0"` list of options"
	echo "Options:"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	python: build with python support"
	echo "	ruby: build with ruby support"
	echo "	netcore: build with netcore support"
	echo "	netcore2: build with netcore 2 support"
	echo "	netcore5: build with netcore 5 support"
	echo "	netcore7: build with netcore 7 support"
	echo "	v8: build with v8 support"
	echo "	nodejs: build with nodejs support"
	echo "	typescript: build with typescript support"
	echo "	file: build with file support"
	echo "	rpc: build with rpc support"
	echo "	wasm: build with wasm support"
	echo "	java: build with java support"
	echo "	c: build with c support"
	echo "	cobol: build with cobol support"
	echo "	go: build with go support"
	echo "	rust: build with rust support"
	echo "	scripts: build all scripts"
	echo "	examples: build all examples"
	echo "	tests: build and run all tests"
	echo "	benchmarks: build and run all benchmarks"
	echo "	install: install all libraries"
	echo "	static: build as static libraries"
	echo "	ports: build all ports"
	echo "	sandbox: build with sandboxing support"
	echo "	coverage: build all coverage reports"
	echo "	address-sanitizer: build with address sanitizer"
	echo "	thread-sanitizer: build with thread sanitizer"
	echo "	memory-sanitizer: build with memory sanitizer"
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
