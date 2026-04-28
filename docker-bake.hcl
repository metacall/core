#
#	MetaCall Library by Parra Studios
#	Docker bake configuration for MetaCall.
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

# Metacall Build Variables
variable "METACALL_BASE_IMAGE" {
	validation {
		condition = METACALL_BASE_IMAGE != ""
		error_message = "Variable 'METACALL_BASE_IMAGE' is required and cannot be empty."
	}
}

variable "METACALL_PATH" {
	validation {
		condition = METACALL_PATH != ""
		error_message = "Variable 'METACALL_PATH' is required."
	}
}

variable "METACALL_BUILD_TYPE" {
	validation {
		condition = METACALL_BUILD_TYPE != ""
		error_message = "Variable 'METACALL_BUILD_TYPE' is required."
	}
}

variable "METACALL_INSTALL_OPTIONS" {
	validation {
		condition = METACALL_INSTALL_OPTIONS != ""
		error_message = "Variable 'METACALL_INSTALL_OPTIONS' is required."
	}
}

variable "METACALL_BUILD_OPTIONS" {
	validation {
		condition = METACALL_BUILD_OPTIONS != ""
		error_message = "Variable 'METACALL_BUILD_OPTIONS' is required."
	}
}

variable "METACALL_RUNTIME_OPTIONS" {
	validation {
		condition = METACALL_RUNTIME_OPTIONS != ""
		error_message = "Variable 'METACALL_RUNTIME_OPTIONS' is required."
	}
}

# Docker Registry Variables
variable "DOCKER_USERNAME" {
	validation {
		condition = DOCKER_USERNAME != ""
		error_message = "Variable 'DOCKER_USERNAME' is required for tagging."
	}
}

variable "IMAGE_NAME" {
	validation {
		condition = IMAGE_NAME != ""
		error_message = "Variable 'IMAGE_NAME' is required."
	}
}

# Build all images in dependency order
group "default" {
	targets = ["deps", "dev", "runtime", "cli"]
}

# Base dependencies image
target "deps" {
	context = "."
	dockerfile = "tools/docker/deps/Dockerfile"
	args = {
		METACALL_BASE_IMAGE = "${METACALL_BASE_IMAGE}"
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_TOOLS_PATH = "${METACALL_PATH}/tools"
		METACALL_BUILD_TYPE = "${METACALL_BUILD_TYPE}"
		METACALL_INSTALL_OPTIONS = "${METACALL_INSTALL_OPTIONS}"
	}
}

# Development image (depends on deps)
target "dev" {
	context = "."
	dockerfile = "tools/docker/dev/Dockerfile"
	args = {
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_BUILD_TYPE = "${METACALL_BUILD_TYPE}"
		METACALL_BUILD_OPTIONS = "${METACALL_BUILD_OPTIONS}"
		METACALL_DEPS_IMAGE = "deps"
	}
	# Use the deps target as the base image
	contexts = {
		"deps" = "target:deps"
	}
}

# Runtime image (depends on dev for builder stage)
target "runtime" {
	context = "."
	dockerfile = "tools/docker/runtime/Dockerfile"
	args = {
		METACALL_BASE_IMAGE = "${METACALL_BASE_IMAGE}"
		METACALL_PATH = "${METACALL_PATH}"
		METACALL_RUNTIME_OPTIONS = "${METACALL_RUNTIME_OPTIONS}"
		METACALL_DEV_IMAGE = "dev"
	}
	# Use the dev target as the builder base image
	contexts = {
		"dev" = "target:dev"
	}
}

# CLI image (depends on dev for builder and runtime for base)
target "cli" {
	context = "."
	dockerfile = "tools/docker/cli/Dockerfile"
	args = {
		METACALL_DEV_IMAGE = "dev"
		METACALL_RUNTIME_IMAGE = "runtime"
	}
	# Use both dev (for builder) and runtime (for base) targets
	contexts = {
		"dev" = "target:dev"
		"runtime" = "target:runtime"
	}
}
