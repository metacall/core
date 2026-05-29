#
#	CMake Find gRPC by Parra Studios
#	CMake script to find gRPC and Protobuf libraries.
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
# GRPC_FOUND          - TRUE if gRPC and Protobuf were found.
# GRPC_INCLUDE_DIRS   - Protobuf include directories (contains google/protobuf/struct.pb.h).
# GRPC_LIBRARIES      - Libraries to link against (protobuf + grpc++).

# Prevent verbosity if already included
if(GRPC_FOUND)
	set(GRPC_FIND_QUIETLY TRUE)
endif()

# Try CMake config first (works with Homebrew, vcpkg, system installs)
find_package(gRPC QUIET CONFIG)

if(gRPC_FOUND)
	set(GRPC_FOUND TRUE)
	set(GRPC_LIBRARIES gRPC::grpc++ protobuf::libprotobuf)

	get_target_property(GRPC_INCLUDE_DIRS protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)

	if(NOT GRPC_FIND_QUIETLY)
		message(STATUS "Found gRPC via CMake config")
	endif()

	return()
endif()

# Fallback: pkg-config
find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(GRPC QUIET grpc++ protobuf)
endif()

# Fallback: manual search
if(NOT GRPC_FOUND)
	find_path(GRPC_INCLUDE_DIRS
		NAMES google/protobuf/struct.pb.h
		PATHS
			/usr/local/include
			/usr/include
			/opt/homebrew/include
		DOC "Include directory for Protobuf (required by grpc_serial)"
	)

	find_library(GRPC_GRPCPP_LIBRARY
		NAMES grpc++ grpc++_unsecure
		PATHS /usr/local/lib /usr/lib /opt/homebrew/lib
	)

	find_library(GRPC_PROTOBUF_LIBRARY
		NAMES protobuf
		PATHS /usr/local/lib /usr/lib /opt/homebrew/lib
	)

	if(GRPC_INCLUDE_DIRS AND GRPC_GRPCPP_LIBRARY AND GRPC_PROTOBUF_LIBRARY)
		set(GRPC_FOUND TRUE)
		set(GRPC_LIBRARIES ${GRPC_GRPCPP_LIBRARY} ${GRPC_PROTOBUF_LIBRARY})
	endif()
endif()

mark_as_advanced(GRPC_FOUND GRPC_INCLUDE_DIRS GRPC_LIBRARIES)

if(GRPC_FOUND)
	if(NOT GRPC_FIND_QUIETLY)
		message(STATUS "Found gRPC/Protobuf headers in ${GRPC_INCLUDE_DIRS}")
	endif()
elseif(GRPC_FIND_REQUIRED)
	message(FATAL_ERROR "Could not find gRPC/Protobuf")
endif()