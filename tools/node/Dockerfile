#
#	MetaCall Library by Parra Studios
#	Docker image infrastructure for MetaCall.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Configure MetaCall Depends node image
ARG METACALL_BASE_IMAGE

# MetaCall Depends node image
FROM ${METACALL_BASE_IMAGE} AS deps_node

# Image descriptor
LABEL copyright.name="Vicente Eduardo Ferrer Garcia" \
	copyright.address="vic798@gmail.com" \
	maintainer.name="Vicente Eduardo Ferrer Garcia" \
	maintainer.address="vic798@gmail.com" \
	vendor="MetaCall Inc." \
	version="0.1"

# Arguments
ARG METACALL_PATH

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall environment script
COPY tools/metacall-environment.sh $METACALL_PATH/tools/

# Install build dependencies
RUN tools/metacall-environment.sh root base nodejs

# Copy MetaCall NodeJS cmake script
COPY cmake/FindNodeJS.cmake $METACALL_PATH/cmake/FindNodeJS.cmake

# Run NodeJS cmake script (TODO: Make find library prefixes/suffixes generic)
ARG METACALL_NODE_BUILD_TYPE

RUN cmake \
		-DCMAKE_BUILD_TYPE=${METACALL_NODE_BUILD_TYPE} \
		-DCMAKE_FIND_LIBRARY_PREFIXES=lib \
		-DCMAKE_FIND_LIBRARY_SUFFIXES=".a;.so" \
		-P $METACALL_PATH/cmake/FindNodeJS.cmake
