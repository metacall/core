#!/usr/bin/env bash

#
#	MetaCall Sanitizer Bash Script by Parra Studios
#	Install, build and sanitizer test bash script utility for MetaCall.
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

set -euxo pipefail

BUILD_SANITIZER=${1:-sanitizer}
BUILD_LANGUAGES=(
	python ruby netcore5 nodejs typescript file rpc wasm java c cobol rust
)
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
ROOT_DIR=$(dirname "$SCRIPT_DIR")
BUILD_DIR="${ROOT_DIR}/build"

if [ "${BUILD_SANITIZER}" != "sanitizer" ] && [ "${BUILD_SANITIZER}" != "thread-sanitizer" ]; then
	echo "Sanitizer '${BUILD_SANITIZER}' not supported, use 'sanitizer' or 'thread-sanitizer'."
	exit 1
fi

export DEBIAN_FRONTEND="noninteractive"
export LTTNG_UST_REGISTER_TIMEOUT=0
export NUGET_XMLDOC_MODE="skip"
export DOTNET_CLI_TELEMETRY_OPTOUT="true"

# Install
"${SCRIPT_DIR}/metacall-environment.sh" base ${BUILD_LANGUAGES[@]} rapidjson funchook swig pack backtrace

# Configure and Build
export NODE_PATH="/usr/lib/node_modules"
export LOADER_LIBRARY_PATH="${BUILD_DIR}"
export LOADER_SCRIPT_PATH="${BUILD_DIR}/scripts"
export CONFIGURATION_PATH="${BUILD_DIR}/configurations/global.json"
export SERIAL_LIBRARY_PATH="${BUILD_DIR}"
export DETOUR_LIBRARY_PATH="${BUILD_DIR}"
export PORT_LIBRARY_PATH="${BUILD_DIR}"

BUILD_OPTIONS=(
	${BUILD_SANITIZER} debug ${BUILD_LANGUAGES[@]} examples tests scripts ports dynamic install pack benchmarks
)

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
"${SCRIPT_DIR}/metacall-configure.sh" ${BUILD_OPTIONS[@]}
"${SCRIPT_DIR}/metacall-build.sh" ${BUILD_OPTIONS[@]}
