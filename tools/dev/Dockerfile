#
#	MetaCall Library by Parra Studios
#	Docker image infrastructure for MetaCall.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# MetaCall Depends image
FROM metacall/core:deps AS dev

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
ENV LOADER_LIBRARY_PATH=$METACALL_PATH/build \
	LOADER_SCRIPT_PATH=$METACALL_PATH/build/scripts \
	CONFIGURATION_PATH=$METACALL_PATH/build/configurations/global.json \
	SERIAL_LIBRARY_PATH=$METACALL_PATH/build \
	DETOUR_LIBRARY_PATH=$METACALL_PATH/build \
	PORT_LIBRARY_PATH=$METACALL_PATH/build \
	DEBIAN_FRONTEND=noninteractive \
	NODE_PATH=/usr/lib/node_modules

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall dependecies
COPY . $METACALL_PATH

# Configure MetaCall build tool script
RUN chmod 500 $METACALL_PATH/tools/metacall-configure.sh \
	&& chmod 500 $METACALL_PATH/tools/metacall-build.sh \
	&& mkdir -p $METACALL_PATH/build

# Configure MetaCall libraries and build runtimes if needed
ARG METACALL_BUILD_TYPE
ARG METACALL_BUILD_OPTIONS

RUN cd $METACALL_PATH/build \
	&& $METACALL_PATH/tools/metacall-configure.sh ${METACALL_BUILD_TYPE} ${METACALL_BUILD_OPTIONS}

# Build test and install MetaCall libraries
RUN cd $METACALL_PATH/build \
	&& $METACALL_PATH/tools/metacall-build.sh ${METACALL_BUILD_TYPE} ${METACALL_BUILD_OPTIONS}
