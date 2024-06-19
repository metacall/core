#
#	MetaCall Library by Parra Studios
#	Docker image infrastructure for MetaCall.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# MetaCall Depends builder image
FROM metacall/core:dev AS builder

# MetaCall image
FROM metacall/core:runtime AS cli

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
	DETOUR_LIBRARY_PATH=/usr/local/lib \
	PORT_LIBRARY_PATH=/usr/local/lib \
	DEBIAN_FRONTEND=noninteractive \
	NODE_PATH=/usr/local/lib/node_modules \
	DOTNET_CLI_TELEMETRY_OPTOUT=true

# Define working directory
WORKDIR $LOADER_SCRIPT_PATH

# Copy cli from builder
COPY --from=builder /usr/local/bin/metacallcli* /usr/local/bin/metacallcli

# Define entry point
ENTRYPOINT [ "metacallcli" ]
