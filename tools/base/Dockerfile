#
# MetaCall Library by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Configure MetaCall base image
ARG METACALL_BASE_IMAGE

# MetaCall base image
FROM ${METACALL_BASE_IMAGE}

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
ARG METACALL_INSTALL_OPTIONS

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
COPY tools/metacall-environment.sh $METACALL_TOOLS_PATH/metacall-environment.sh
COPY tools/nobuildtest.patch $METACALL_TOOLS_PATH/nobuildtest.patch

# Configure MetaCall environment tool script
RUN chmod 500 $METACALL_TOOLS_PATH/metacall-environment.sh

# Install MetaCall and runtimes then build
RUN apt-get update \
	&& $METACALL_TOOLS_PATH/metacall-environment.sh ${METACALL_INSTALL_OPTIONS}

# Delete MetaCall tools path
RUN rm -rf $METACALL_PATH
