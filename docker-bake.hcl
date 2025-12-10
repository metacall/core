#
#	MetaCall Library by Parra Studios
#	Docker bake configuration for MetaCall.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Variables from environment
variable "METACALL_BASE_IMAGE" {
	default = "debian:trixie-slim"
}

variable "METACALL_PATH" {
	default = "/usr/local/metacall"
}

variable "METACALL_BUILD_TYPE" {
	default = "relwithdebinfo"
}

variable "METACALL_INSTALL_OPTIONS" {
	default = "base python ruby nodejs typescript file rpc rapidjson pack backtrace"
}

variable "METACALL_BUILD_OPTIONS" {
	default = "python ruby nodejs typescript file rpc examples tests scripts ports install pack"
}

variable "METACALL_RUNTIME_OPTIONS" {
	default = "base python ruby nodejs typescript file rpc backtrace ports clean"
}

variable "DOCKER_USERNAME" {
	default = "metacall"
}

variable "IMAGE_NAME" {
	default = "core"
}

# Build all images in dependency order
group "default" {
	targets = ["deps", "dev", "runtime", "cli"]
}

# Base dependencies image
target "deps" {
	context = "."
	dockerfile = "tools/deps/Dockerfile"
	tags = ["${DOCKER_USERNAME}/${IMAGE_NAME}:deps"]
	args = {
		METACALL_BASE_IMAGE = "${METACALL_BASE_IMAGE}"
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_TOOLS_PATH = "${METACALL_PATH}/tools"
		METACALL_BUILD_TYPE = "${METACALL_BUILD_TYPE}"
		METACALL_INSTALL_OPTIONS = "${METACALL_INSTALL_OPTIONS}"
	}
	network = "host"
}

# Development image (depends on deps)
target "dev" {
	context = "."
	dockerfile = "tools/dev/Dockerfile"
	tags = ["${DOCKER_USERNAME}/${IMAGE_NAME}:dev"]
	args = {
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_BUILD_TYPE = "${METACALL_BUILD_TYPE}"
		METACALL_BUILD_OPTIONS = "${METACALL_BUILD_OPTIONS}"
	}
	network = "host"
	# Use the deps target as the base image
	contexts = {
		"metacall/core:deps" = "target:deps"
	}
}

# Runtime image (depends on dev for builder stage)
target "runtime" {
	context = "."
	dockerfile = "tools/runtime/Dockerfile"
	tags = ["${DOCKER_USERNAME}/${IMAGE_NAME}:runtime"]
	args = {
		METACALL_BASE_IMAGE = "${METACALL_BASE_IMAGE}"
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_RUNTIME_OPTIONS = "${METACALL_RUNTIME_OPTIONS}"
	}
	network = "host"
	# Use the dev target as the builder base image
	contexts = {
		"metacall/core:dev" = "target:dev"
	}
}

# CLI image (depends on dev for builder and runtime for base)
target "cli" {
	context = "."
	dockerfile = "tools/cli/Dockerfile"
	tags = ["${DOCKER_USERNAME}/${IMAGE_NAME}:cli"]
	network = "host"
	# Use both dev (for builder) and runtime (for base) targets
	contexts = {
		"metacall/core:dev" = "target:dev"
		"metacall/core:runtime" = "target:runtime"
	}
}
