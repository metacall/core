#!/usr/bin/env sh

#
#	MetaCall Android Emulator Shell Script by Parra Studios
#	Cross-compiling emulator for running MetaCall tests on Android through adb.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# This script is used as CMAKE_CROSSCOMPILING_EMULATOR, CTest runs it as:
#	metacall-android-emulator.sh <host source dir> <host build dir> <device dir> <executable> [arguments...]
#
# The build directory must be already pushed to the device (<device dir>/build), the host
# paths of the executable, arguments, working directory and environment variables are
# mapped into the device paths before running it with adb shell.
#
# Environment variables:
#	ANDROID_EMULATOR_ENV_FILE: file with the test environment variables (one VAR=value per line)

set -eu

if [ $# -lt 4 ]; then
	echo "Usage: $(basename "$0") <host source dir> <host build dir> <device dir> <executable> [arguments...]" >&2
	exit 1
fi

HOST_SOURCE_DIR="$1"
HOST_BUILD_DIR="$2"
DEVICE_DIR="$3"
shift 3

DEVICE_SOURCE_DIR="$DEVICE_DIR/source"
DEVICE_BUILD_DIR="$DEVICE_DIR/build"

# Escape a string for using it as a sed regular expression
sub_escape_regex() {
	printf '%s' "$1" | sed -e 's/[]\/$*.^|[]/\\&/g'
}

# Escape a string for using it as a sed replacement
sub_escape_replacement() {
	printf '%s' "$1" | sed -e 's/[\/&|]/\\&/g'
}

HOST_SOURCE_REGEX=$(sub_escape_regex "$HOST_SOURCE_DIR")
HOST_BUILD_REGEX=$(sub_escape_regex "$HOST_BUILD_DIR")
DEVICE_SOURCE_REPLACEMENT=$(sub_escape_replacement "$DEVICE_SOURCE_DIR")
DEVICE_BUILD_REPLACEMENT=$(sub_escape_replacement "$DEVICE_BUILD_DIR")

# Map host paths into device paths (build directory first because it can be inside the source directory)
sub_map_path() {
	printf '%s' "$1" | sed \
		-e "s|$HOST_BUILD_REGEX|$DEVICE_BUILD_REPLACEMENT|g" \
		-e "s|$HOST_SOURCE_REGEX|$DEVICE_SOURCE_REPLACEMENT|g"
}

# Quote a string for the device shell
sub_quote() {
	printf "'%s'" "$(printf '%s' "$1" | sed -e "s/'/'\\\\''/g")"
}

# Working directory, if it is outside of the mapped directories use the build directory
DEVICE_WORKING_DIR=$(sub_map_path "$PWD")

if [ "$DEVICE_WORKING_DIR" = "$PWD" ]; then
	DEVICE_WORKING_DIR="$DEVICE_BUILD_DIR"
fi

COMMAND="cd $(sub_quote "$DEVICE_WORKING_DIR") && env $(sub_quote "LD_LIBRARY_PATH=$DEVICE_BUILD_DIR")"

# Load the test environment variables
if [ -n "${ANDROID_EMULATOR_ENV_FILE:-}" ]; then
	while IFS= read -r line || [ -n "$line" ]; do
		if [ -n "$line" ]; then
			COMMAND="$COMMAND $(sub_quote "$(sub_map_path "$line")")"
		fi
	done < "$ANDROID_EMULATOR_ENV_FILE"
fi

# Executable and arguments
for arg in "$@"; do
	COMMAND="$COMMAND $(sub_quote "$(sub_map_path "$arg")")"
done

# The adb shell protocol returns the exit code of the command
exec adb shell "$COMMAND"
