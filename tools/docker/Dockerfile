#
#	MetaCall Library by Parra Studios
#	Docker image infrastructure for MetaCall.
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

# Configure MetaCall Depends node image
ARG METACALL_BASE_IMAGE=debian:trixie-slim

# MetaCall Depends image
FROM ${METACALL_BASE_IMAGE} AS deps

# Image descriptor
LABEL copyright.name="Vicente Eduardo Ferrer Garcia" \
	copyright.address="vic798@gmail.com" \
	maintainer.name="Vicente Eduardo Ferrer Garcia" \
	maintainer.address="vic798@gmail.com" \
	vendor="MetaCall Inc." \
	version="0.1"

# Arguments
ARG METACALL_PATH
ARG METACALL_TOOLS_PATH

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall tool dependecies
COPY tools/metacall-environment.sh tools/nobuildtest.patch $METACALL_TOOLS_PATH/

# Install MetaCall and runtimes then build
ARG METACALL_BUILD_TYPE
ARG METACALL_INSTALL_OPTIONS

RUN chmod 500 $METACALL_TOOLS_PATH/metacall-environment.sh \
	&& $METACALL_TOOLS_PATH/metacall-environment.sh ${METACALL_BUILD_TYPE} ${METACALL_INSTALL_OPTIONS} \
	&& rm -rf $METACALL_PATH

# MetaCall Dev image
FROM deps AS dev

# Arguments
ARG METACALL_PATH

# Environment variables
ENV LOADER_LIBRARY_PATH=$METACALL_PATH/build \
	LOADER_SCRIPT_PATH=$METACALL_PATH/build/scripts \
	CONFIGURATION_PATH=$METACALL_PATH/build/configurations/global.json \
	SERIAL_LIBRARY_PATH=$METACALL_PATH/build \
	DETOUR_LIBRARY_PATH=$METACALL_PATH/build

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

# MetaCall Builder image
FROM dev AS builder

# Arguments
ARG METACALL_PATH

# Copy all installed files for export them later on in Runtime image
# Skip the CLI because we will copy it at the end in the CLI image
RUN mkdir -p /dist \
	&& while IFS= read -r file; do \
		[ -e "$file" ] || continue; \
		case "$file" in \
			/usr/local/bin/metacallcli|/usr/local/bin/metacallclid) continue ;; \
		esac; \
		mkdir -p "/dist/$(dirname "${file#/}")"; \
		cp -a "$file" "/dist/${file#/}"; \
	done < "$METACALL_PATH/build/install_manifest.txt"

# MetaCall Runtime image
FROM ${METACALL_BASE_IMAGE} AS runtime

# Image descriptor
LABEL copyright.name="Vicente Eduardo Ferrer Garcia" \
	copyright.address="vic798@gmail.com" \
	maintainer.name="Vicente Eduardo Ferrer Garcia" \
	maintainer.address="vic798@gmail.com" \
	vendor="MetaCall Inc." \
	version="0.1"

# Environment variables
ENV LOADER_LIBRARY_PATH=/usr/local/lib \
	LOADER_SCRIPT_PATH=/usr/local/scripts \
	CONFIGURATION_PATH=/usr/local/share/metacall/configurations/global.json \
	SERIAL_LIBRARY_PATH=/usr/local/lib \
	DETOUR_LIBRARY_PATH=/usr/local/lib

# Arguments
ARG METACALL_PATH

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall dependecies
COPY tools/metacall-runtime.sh $METACALL_PATH

# Configure MetaCall runtime tool script
ARG METACALL_RUNTIME_OPTIONS

# Install runtimes
RUN mkdir -p /usr/local/scripts \
	&& chmod 500 $METACALL_PATH/metacall-runtime.sh \
	&& $METACALL_PATH/metacall-runtime.sh ${METACALL_RUNTIME_OPTIONS} \
	&& rm -rf $METACALL_PATH/metacall-runtime.sh

# Copy all files from builder
COPY --from=builder /dist/ /

# MetaCall CLI image
FROM runtime AS cli

# Define working directory
WORKDIR $LOADER_SCRIPT_PATH

# Copy cli from dev
COPY --from=dev /usr/local/bin/metacallcli* /usr/local/bin/metacallcli

# Define entry point
ENTRYPOINT [ "metacallcli" ]
