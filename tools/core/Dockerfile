#
# MetaCall Library by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# MetaCall Depends node builder image
FROM metacall/core_deps_node AS builder_node

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

# MetaCall Depends image
FROM metacall/core_deps

# Image descriptor
LABEL copyright.name="Vicente Eduardo Ferrer Garcia" \
	copyright.address="vic798@gmail.com" \
	maintainer.name="Vicente Eduardo Ferrer Garcia" \
	maintainer.address="vic798@gmail.com" \
	vendor="MetaCall Inc." \
	version="0.1"

# Arguments
ARG METACALL_PATH
ARG METACALL_BUILD_TYPE
ARG METACALL_BUILD_OPTIONS

# Environment variables
ENV LOADER_LIBRARY_PATH=$METACALL_PATH/build \
	LOADER_SCRIPT_PATH=$METACALL_PATH/build/scripts \
	CONFIGURATION_PATH=$METACALL_PATH/build/configurations/global.json \
	SERIAL_LIBRARY_PATH=$METACALL_PATH/build \
	DEBIAN_FRONTEND=noninteractive

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall NodeJS dependencies (TODO: Make location fixed)
COPY --from=builder_node /usr/local/lib/libnode.so.* /usr/local/lib/

# Copy MetaCall dependecies
COPY . $METACALL_PATH

# Configure MetaCall build tool script
RUN chmod 500 $METACALL_PATH/tools/metacall-configure.sh \
	&& chmod 500 $METACALL_PATH/tools/metacall-build.sh \
	&& chmod 500 $METACALL_PATH/tools/metacall-clear.sh \
	&& mkdir -p $METACALL_PATH/build

# Configure MetaCall libraries and build runtimes if needed
RUN cd $METACALL_PATH/build \
	&& $METACALL_PATH/tools/metacall-configure.sh ${METACALL_BUILD_TYPE} ${METACALL_BUILD_OPTIONS}

# Build and install MetaCall libraries then run tests
RUN cd $METACALL_PATH/build \
	&& $METACALL_PATH/tools/metacall-build.sh ${METACALL_BUILD_TYPE} ${METACALL_BUILD_OPTIONS}

# Clear MetaCall build dependencies
RUN cd $METACALL_PATH/build \
	&& $METACALL_PATH/tools/metacall-clear.sh ${METACALL_BUILD_TYPE} ${METACALL_BUILD_OPTIONS}
