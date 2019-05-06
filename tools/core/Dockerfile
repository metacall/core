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

# Configure MetaCall base image
ARG METACALL_BASE_IMAGE

# MetaCall Depends builder image
FROM metacall/core:dev AS builder

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

# MetaCall image
FROM ${METACALL_BASE_IMAGE} AS core

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
ENV LOADER_LIBRARY_PATH=/usr/local/lib \
	LOADER_SCRIPT_PATH=/usr/local/scripts \
	CONFIGURATION_PATH=/usr/local/share/metacall/configurations/global.json \
	SERIAL_LIBRARY_PATH=/usr/local/lib \
	DETOUR_LIBRARY_PATH=/usr/local/lib \
	PORT_LIBRARY_PATH=/usr/local/lib \
	DEBIAN_FRONTEND=noninteractive

# Define working directory
WORKDIR $METACALL_PATH

# Copy libraries from builder
COPY --from=builder /usr/local/lib/*.so /usr/local/lib/*.so* /usr/local/lib/*.dll /usr/local/lib/*.js /usr/local/lib/*.node /usr/local/lib/

# TODO: Remove this when metacall is able to be installed from package managers (pip, npm, gem)
COPY --from=builder /usr/local/lib/python3.5/dist-packages/ /usr/local/lib/python3.5/dist-packages/

# Copy MetaCall dependecies
COPY tools/metacall-runtime.sh $METACALL_PATH

# Configure MetaCall runtime tool script
ARG METACALL_RUNTIME_OPTIONS

# Install runtimes
RUN mkdir -p /usr/local/scripts \
	&& chmod 500 $METACALL_PATH/metacall-runtime.sh \
	&& $METACALL_PATH/metacall-runtime.sh ${METACALL_RUNTIME_OPTIONS} \
	&& rm -rf $METACALL_PATH/metacall-runtime.sh

# Copy headers from builder
COPY --from=builder /usr/local/include/metacall /usr/local/include/metacall

# Copy configuration from builder
COPY --from=builder /usr/local/share/metacall/configurations/* /usr/local/share/metacall/configurations/

