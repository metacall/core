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
	$Global:ExitCode = 0

	# Build the project
	echo "Building MetaCall..."
	cmake --build . "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)"

	if ( -not $? ) {
		$RecentExitCode = $LASTEXITCODE
		echo "Failure in build with exit code: $RecentExitCode"

		$Global:ExitCode = $RecentExitCode
	}

	# Tests (coverage needs to run the tests)
	if ( ($BUILD_TESTS -eq 1) -or ($BUILD_COVERAGE -eq 1) ) {
		echo "Running the tests..."

		# Prerequisites
		$files = @(
			"C:\Program Files\nodejs\lib\libnode.dll",
			".\runtimes\ruby\bin\x64-vcruntime140-ruby310.dll"
		)

		ForEach ($file in $files) {
			if ( (Test-Path $file -PathType Leaf) ) {
				echo "Copying ""$file"" to "".\$BUILD_TYPE\""..."
				cp $file ".\$BUILD_TYPE\"
			}
		}

		ctest "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)" --timeout 5400 --output-on-failure -C $BUILD_TYPE

		if ( -not $? ) {
			$RecentExitCode = $LASTEXITCODE
			echo "Failure in tests with exit code: $RecentExitCode"

			$Global:ExitCode = $RecentExitCode
		}
	}

	# Coverage
	<# if ( $BUILD_COVERAGE = 1 ) {
		# TODO (copied): Remove -k, solve coverage issues
		# TODO: Migrate to Windows
		echo "Reporting coverage..."
		make -k gcov
		make -k lcov
		make -k lcov-genhtml

		if ( -not $? ) {
			$RecentExitCode = $LASTEXITCODE
			echo "Failure in coverage with exit code: $RecentExitCode"

			$Global:ExitCode = $RecentExitCode
		}
	} #>

	# Install
	if ( $BUILD_INSTALL -eq 1 ) {
		echo "Building and installing MetaCall..."
		cmake --build . --target install

		if ( -not $? ) {
			$RecentExitCode = $LASTEXITCODE
			echo "Failure in install with exit code: $RecentExitCode"

			$Global:ExitCode = $RecentExitCode
		}
	}

	Exit $ExitCode
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
