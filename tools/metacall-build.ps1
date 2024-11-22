<#
#	MetaCall Build PowerShell Script by Parra Studios
#	Build and install powershell script utility for MetaCall.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#>

$Global:BUILD_TYPE       = 'Release'
$Global:BUILD_TESTS      = 0
$Global:BUILD_BENCHMARKS = 0
$Global:BUILD_COVERAGE   = 0
$Global:BUILD_INSTALL    = 0

$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename
$Global:Arguments = $args

function Sub-Options {
	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$option = $Arguments[$i]
		if ("$option" -eq "debug") {
			echo "Build all scripts in debug mode"
			$Global:BUILD_TYPE = 'Debug'
		}
		if ("$option" -eq "release") {
			echo "Build all scripts in release mode"
			$Global:BUILD_TYPE = 'Release'
		}
		if ("$option" -eq "relwithdebinfo") {
			echo "Build all scripts in release mode with debug symbols"
			$Global:BUILD_TYPE = 'RelWithDebInfo'
		}
		if ("$option" -eq "tests") {
			echo "Build and run all tests"
			$Global:BUILD_TESTS = 1
		}
		if ("$option" -eq "benchmarks") {
			echo "Build and run all benchmarks"
			$Global:BUILD_BENCHMARKS = 1
		}
		if ("$option" -eq "install") {
			echo "Install all libraries"
			$Global:BUILD_INSTALL = 1
		}
	}
}

function Sub-Build {
	$Global:ExitCode = 0

	# Build the project
	echo "Building MetaCall..."
	cmake --build . "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)" --config $BUILD_TYPE

	if (-not $?) {
		$RecentExitCode = $LASTEXITCODE
		echo "Failure in build with exit code: $RecentExitCode"

		$Global:ExitCode = $RecentExitCode
		Exit $ExitCode
	}

	# Tests
	if (($BUILD_TESTS -eq 1) -or ($BUILD_BENCHMARKS -eq 1) -or ($BUILD_COVERAGE -eq 1)) {
		echo "Running the tests..."
		ctest "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)" --timeout 5400 --output-on-failure -C $BUILD_TYPE

		if (-not $?) {
			$RecentExitCode = $LASTEXITCODE
			echo "Failure in tests with exit code: $RecentExitCode"

			$Global:ExitCode = $RecentExitCode
			Exit $ExitCode
		}
	}

	# Install
	if ($BUILD_INSTALL -eq 1) {
		echo "Building and installing MetaCall..."
		cmake --build . --target install --config $BUILD_TYPE

		if (-not $?) {
			$RecentExitCode = $LASTEXITCODE
			echo "Failure in install with exit code: $RecentExitCode"

			$Global:ExitCode = $RecentExitCode
			Exit $ExitCode
		}
	}

	Exit $ExitCode
}

function Sub-Help {
	echo "Usage: $PROGNAME list of options"
	echo "Options:"
	echo "	debug | release | relwithdebinfo: build type"
	echo "	tests: build and run all tests"
	echo "	install: install all libraries"
	echo ""
}

switch($args.length) {
	0 {
		Sub-Help
		Break
	}
	Default {
		Sub-Options
		Sub-Build
	}
}
