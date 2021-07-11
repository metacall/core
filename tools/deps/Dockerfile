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

# Configure MetaCall Depends node image
ARG METACALL_BASE_IMAGE

# MetaCall Depends base image
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

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive \
	LTTNG_UST_REGISTER_TIMEOUT=0 \
	NUGET_XMLDOC_MODE=skip \
	DOTNET_CLI_TELEMETRY_OPTOUT=true

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall tool dependecies
COPY tools/metacall-environment.sh tools/nobuildtest.patch $METACALL_TOOLS_PATH/

# Install MetaCall and runtimes then build
ARG METACALL_INSTALL_OPTIONS

RUN chmod 500 $METACALL_TOOLS_PATH/metacall-environment.sh \
	&& $METACALL_TOOLS_PATH/metacall-environment.sh ${METACALL_INSTALL_OPTIONS} \
	&& rm -rf $METACALL_PATH
