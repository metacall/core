$Global:ROOT_DIR = "$(pwd)"

$Global:BUILD_TYPE =       'Release'
$Global:BUILD_PYTHON =     0
$Global:BUILD_RUBY =       0
$Global:BUILD_NETCORE =    0
$Global:BUILD_NETCORE2 =   0
$Global:BUILD_NETCORE5 =   0
$Global:BUILD_V8 =         0
$Global:BUILD_NODEJS =     0
$Global:BUILD_TYPESCRIPT = 0
$Global:BUILD_RUST =       0
$Global:BUILD_FILE =       0
$Global:BUILD_RPC =        0
$Global:BUILD_WASM =       0
$Global:BUILD_JAVA =       0
$Global:BUILD_C =          0
$Global:BUILD_COBOL =      0
$Global:BUILD_SCRIPTS =    0
$Global:BUILD_EXAMPLES =   0
$Global:BUILD_TESTS =      0
$Global:BUILD_BENCHMARKS = 0
$Global:BUILD_PORTS =      0
$Global:BUILD_COVERAGE =   0
$Global:BUILD_SANITIZER =  0
$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename

$Global:Arguments = $args

function sub-options {
	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$option = $Arguments[$i]
		if ( "$option" -eq 'debug' ) {
			echo "Build all scripts in debug mode"
			$Global:BUILD_TYPE = 'Debug'
		}
		if ( "$option" -eq 'release' ) {
			echo "Build all scripts in release mode"
			$Global:BUILD_TYPE = 'Release'
		}
		if ( "$option" -eq 'relwithdebinfo' ) {
			echo "Build all scripts in release mode with debug symbols"
			$Global:BUILD_TYPE = 'RelWithDebInfo'
		}
		if ( "$option" -eq 'python' ) {
			echo "Build with python support"
			$Global:BUILD_PYTHON = 1
		}
		if ( "$option" -eq 'ruby' ) {
			echo "Build with ruby support"
			$Global:BUILD_RUBY = 1
		}
		if ( "$option" -eq 'netcore' ) {
			echo "Build with netcore support"
			$Global:BUILD_NETCORE = 1
		}
		if ( "$option" -eq 'netcore2' ) {
			echo "Build with netcore 2 support"
			$Global:BUILD_NETCORE2 = 1
		}
		if ( "$option" -eq 'netcore5' ) {
			echo "Build with netcore 5 support"
			$Global:BUILD_NETCORE5 = 1
		}
		if ( "$option" -eq 'v8' ) {
			echo "Build with v8 support"
			$Global:BUILD_V8 = 1
		}
		if ( "$option" -eq 'nodejs' ) {
			echo "Build with nodejs support"
			$Global:BUILD_NODEJS = 1
		}
		if ( "$option" -eq 'typescript' ) {
			echo "Build with typescript support"
			$Global:BUILD_TYPESCRIPT = 1
		}
		if ( "$option" -eq 'file' ) {
			echo "Build with file support"
			$Global:BUILD_FILE = 1
		}
		if ( "$option" -eq 'rpc' ) {
			echo "Build with rpc support"
			$Global:BUILD_RPC = 1
		}
		if ( "$option" -eq 'wasm' ) {
			echo "Build with wasm support"
			$Global:BUILD_WASM = 1
		}
		if ( "$option" -eq 'java' ) {
			echo "Build with java support"
			$Global:BUILD_JAVA = 1
		}
		if ( "$option" -eq 'c' ) {
			echo "Build with c support"
			$Global:BUILD_C = 1
		}
		if ( "$option" -eq 'cobol' ) {
			echo "Build with cobol support"
			$Global:BUILD_COBOL = 1
		}
		if ( "$option" -eq 'scripts' ) {
			echo "Build all scripts"
			$Global:BUILD_SCRIPTS = 1
		}
		if ( "$option" -eq 'examples' ) {
			echo "Build all examples"
			$Global:BUILD_EXAMPLES = 1
		}
		if ( "$option" -eq 'tests' ) {
			echo "Build all tests"
			$Global:BUILD_TESTS = 1
		}
		if ( "$option" -eq 'benchmarks' ) {
			echo "Build all benchmarks"
			$Global:BUILD_BENCHMARKS = 1
		}
		if ( "$option" -eq 'ports' ) {
			echo "Build all ports"
			$Global:BUILD_PORTS = 1
		}
		if ( "$option" -eq 'coverage' ) {
			echo "Build all coverage reports"
			$Global:BUILD_COVERAGE = 1
		}
		if ( "$option" -eq 'sanitizer' ) {
			echo "Build with sanitizers"
			$Global:BUILD_SANITIZER = 1
		}
		if ( "$option" -eq 'rust' ) {
			echo "Build with rust support"
			$Global:BUILD_RUST = 1
		}
	}
}

function sub-configure {
	$Global:BUILD_STRING = "-DOPTION_BUILD_LOG_PRETTY=Off " `
			+ "-DOPTION_BUILD_LOADERS=On "                  `
			+ "-DOPTION_BUILD_LOADERS_MOCK=On"

	# Scripts
	if ( $BUILD_SCRIPTS -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS=Off"
	}

	# Python
	if ( $BUILD_PYTHON -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_PY=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_PY=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_PY=On"
		}
	}

	# Ruby
	if ( $BUILD_RUBY -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_RB=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RB=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_RB=On"
		}
	}

	# NetCore
	if ( $BUILD_NETCORE -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING " `
			+ "-DOPTION_BUILD_LOADERS_CS=On " `
			+ "-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.10/"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		}
	}

	# NetCore 2
	if ( $BUILD_NETCORE2 -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING " `
			+ "-DOPTION_BUILD_LOADERS_CS=On " `
			+ "-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/2.2.8/"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		}
	}

	# NetCore 5
	if ( $BUILD_NETCORE5 -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING " `
			+ "-DOPTION_BUILD_LOADERS_CS=On " `
			+ "-DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/5.0.17/"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_CS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_CS=On"
		}
	}

	# V8
	if ( $BUILD_V8 -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_JS=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_JS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_JS=On"
		}
	}

	# NodeJS
	if ( $BUILD_NODEJS -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_NODE=On"

		& {
			cd ..
			$Global:BUILD_STRING = "$BUILD_STRING -DNPM_ROOT=""$($(pwd).Path.Replace('\', '/'))/runtimes/nodejs"""
		}

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_NODE=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_NODE=On"
		}
	}

	# TypeScript
	if ( $BUILD_TYPESCRIPT -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_TS=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_TS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_TS=On"
		}
	}

	# File
	if ( $BUILD_FILE -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_FILE=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_FILE=On"
		}
	}

	# RPC
	if ( $BUILD_RPC -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_RPC=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RPC=On"
		}
	}

	# WebAssembly
	if ( $BUILD_WASM -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_WASM=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_WASM=On"
		}
	}

	# Java
	if ( $BUILD_JAVA -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_JAVA=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_JAVA=On"
		}
	}

	# C
	if ( $BUILD_C -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_C=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_C=On"
		}
	}

	# Cobol
	if ( $BUILD_COBOL -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_COB=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_COB=On"
		}
	}

	# Examples
	if ( $BUILD_EXAMPLES -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_EXAMPLES=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_EXAMPLES=Off"
	}

	# Tests
	if ( $BUILD_TESTS -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_TESTS=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_TESTS=Off"
	}

	# Benchmarks
	if ( $BUILD_BENCHMARKS -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_BENCHMARKS=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_BENCHMARKS=Off"
	}

	# Ports
	if ( $BUILD_PORTS -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS=Off"
	}

	# Coverage
	if ( $BUILD_COVERAGE -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_COVERAGE=On"
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_COVERAGE=Off"
	}

	# Sanitizer
	if ( $BUILD_SANITIZER -eq 1 ) {
		# Disable backtrace module when sanitizer is enabled
		# in order to let the sanitizer catch the segmentation faults
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SANITIZER=On" # -DOPTION_BUILD_BACKTRACE=Off
	} else {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SANITIZER=Off"
	}

	# Rust
	if ( $BUILD_RUST -eq 1 ) {
		$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_LOADERS_RS=On"

		if ( $BUILD_SCRIPTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_SCRIPTS_RS=On"
		}

		if ( $BUILD_PORTS -eq 1 ) {
			$Global:BUILD_STRING = "$BUILD_STRING -DOPTION_BUILD_PORTS_RS=On"
		}
	}

	# Build type
	$Global:BUILD_STRING = "$BUILD_STRING -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
	
	# Execute CMake
	# cmd.exe /c "cmake -Wno-dev -DOPTION_GIT_HOOKS=Off $BUILD_STRING .."
	$CustomFlags = '-DOPTION_BUILD_SECURITY=OFF -DOPTION_FORK_SAFE=OFF'
	echo "BUILD COMMAND: cmake $CustomFlags $BUILD_STRING .."
	cmd.exe /c "cmake $CustomFlags $BUILD_STRING .."

	Exit $LASTEXITCODE
}

function sub-help {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
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
	echo "	cobol: build with cobol support"
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

switch($args.length) {
	0 {
		sub-help
		break
	}
	Default {
		sub-options
		sub-configure
	}
}
