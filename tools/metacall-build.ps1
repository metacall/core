$Global:BUILD_TYPE     = 'Release'
$Global:BUILD_TESTS    = 0
$Global:BUILD_COVERAGE = 0
$Global:BUILD_INSTALL  = 0

$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename
$Global:Arguments = $args

function sub-options {
	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$option = $Arguments[$i]
		if ( "$option" -eq "debug" ) {
			echo "Build all scripts in debug mode"
			$Global:BUILD_TYPE = 'Debug'
		}
		if ( "$option" -eq "release" ) {
			echo "Build all scripts in release mode"
			$Global:BUILD_TYPE = 'Release'
		}
		if ( "$option" -eq "relwithdebinfo" ) {
			echo "Build all scripts in release mode with debug symbols"
			$Global:BUILD_TYPE = 'RelWithDebInfo'
		}
		if ( "$option" -eq "tests" ) {
			echo "Build and run all tests"
			$Global:BUILD_TESTS = 1
		}
		if ( "$option" -eq "coverage" ) {
			echo "Build coverage reports"
			$Global:BUILD_COVERAGE = 1
		}
		if ( "$option" -eq "install" ) {
			echo "Install all libraries"
			$Global:BUILD_INSTALL = 1
		}
	}
}

function sub-build {

	# Build the project
	echo "Building MetaCall..."
	cmake --build . "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)"

	# Tests (coverage needs to run the tests)
	if ( ($BUILD_TESTS -eq 1) -or ($BUILD_COVERAGE -eq 1) ) {
		echo "Running the tests..."
		ctest "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)" --output-on-failure -C $BUILD_TYPE
	}

	# Coverage
	<# if ( $BUILD_COVERAGE = 1 ) {
		# TODO (copied): Remove -k, solve coverage issues
		# TODO: Migrate to Windows
		echo "Reporting coverage..."
		make -k gcov
		make -k lcov
		make -k lcov-genhtml
	} #>

	# Install
	if ( $BUILD_INSTALL -eq 1 ) {
		echo "Building and installing MetaCall..."
		cmake --build . --target install
	}
}

function sub-help {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	tests: build and run all tests"
	echo "	coverage: build coverage reports"
	echo "	install: install all libraries"
	echo ""
}

switch($args.length) {
	0 {
		sub-help
		Break
	}
	Default {
		sub-options
		sub-build
	}
}
