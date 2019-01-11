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

# MetaCall base image
FROM metacall/core_deps_node AS core_deps

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
ENV DEBIAN_FRONTEND=noninteractive

# Work around https://github.com/dotnet/cli/issues/1582 until Docker releases a
# fix (https://github.com/docker/docker/issues/20818). This workaround allows
# the container to be run with the default seccomp Docker settings by avoiding
# the restart_syscall made by LTTng which causes a failed assertion.
ENV LTTNG_UST_REGISTER_TIMEOUT=0

# Trigger the population of the local NuGet package cache
ENV NUGET_XMLDOC_MODE=skip

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall tool dependecies
COPY tools/metacall-environment.sh tools/nobuildtest.patch $METACALL_TOOLS_PATH/

# Install MetaCall and runtimes then build
ARG METACALL_INSTALL_OPTIONS

RUN chmod 500 $METACALL_TOOLS_PATH/metacall-environment.sh \
	&& $METACALL_TOOLS_PATH/metacall-environment.sh ${METACALL_INSTALL_OPTIONS} \
	&& rm -rf $METACALL_PATH
