#
#	CMake Install gRPC by Parra Studios
#	CMake script to install gRPC and Protobuf via FetchContent.
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

# The following variables are set:
#
# GRPC_FOUND        - TRUE if gRPC was installed successfully.
# GRPC_INCLUDE_DIRS - Include directories for Protobuf headers.
# GRPC_LIBRARIES    - Libraries to link against.

if(NOT GRPC_FOUND OR USE_BUNDLED_GRPC)
	include(FetchContent)

	# Pin to a specific release tag for reproducibility
	if(NOT GRPC_VERSION OR USE_BUNDLED_GRPC)
		set(GRPC_VERSION v1.60.0)
	endif()

	message(STATUS "Installing gRPC ${GRPC_VERSION} via FetchContent (this may take a while)")

	# Disable building unnecessary gRPC components to keep build times reasonable
	set(FETCHCONTENT_QUIET OFF)
	set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_CODEGEN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_CPP_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF CACHE BOOL "" FORCE)
	set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF CACHE BOOL "" FORCE)
	set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(gRPC
		GIT_REPOSITORY https://github.com/grpc/grpc.git
		GIT_TAG        ${GRPC_VERSION}
		GIT_SUBMODULES_RECURSE ON
	)

	FetchContent_MakeAvailable(gRPC)

	set(GRPC_FOUND TRUE)
	set(GRPC_LIBRARIES gRPC::grpc++ protobuf::libprotobuf)

	get_target_property(GRPC_INCLUDE_DIRS protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)

	mark_as_advanced(GRPC_INCLUDE_DIRS GRPC_LIBRARIES)

	message(STATUS "Installed gRPC ${GRPC_VERSION}")
endif()