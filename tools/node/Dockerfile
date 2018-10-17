#
# MetaCall Library by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# MetaCall Depends node image
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

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Define working directory
WORKDIR $METACALL_PATH

# Copy MetaCall dependecies
COPY cmake/FindNodeJS.cmake $METACALL_PATH/cmake/FindNodeJS.cmake

# Run NodeJS cmake script (TODO: Make find library prefixes/suffixes generic)
ARG METACALL_BUILD_TYPE_NAME

RUN mkdir -p $METACALL_PATH/build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=${METACALL_BUILD_TYPE_NAME} \
		-DCMAKE_FIND_LIBRARY_PREFIXES=lib \
		-DCMAKE_FIND_LIBRARY_SUFFIXES=".a;.so" \
		-P $METACALL_PATH/cmake/FindNodeJS.cmake
