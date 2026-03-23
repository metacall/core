#!/usr/bin/env bash

#
#	MetaCall Ruby Port Deploy Script by Parra Studios
#	Script utility for deploying MetaCall Ruby Port to RubyGems.
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

set -exuo pipefail

# Move to the package directory
cd "$(dirname "$0")/package"

# Extract the version from the gemspec file
PORT_VERSION=$(ruby -e 'spec = Gem::Specification.load("metacall.gemspec"); puts spec.version')

# Check if the version already exists on RubyGems
# We use the public API to check existing versions
# Returns a list of versions, we grep for an exact match
REMOTE_VERSIONS=$(curl -s https://rubygems.org/api/v1/versions/metacall.json | grep -o '"number":"[^"]*"' | cut -d'"' -f4 || echo "")

if echo "$REMOTE_VERSIONS" | grep -q "^$PORT_VERSION$"; then
	echo "Version ${PORT_VERSION} already exists on RubyGems, skipping upload."
	exit 0
fi

if [[ -z "${RUBYGEMS_API_KEY:-}" ]]; then
	echo "RUBYGEMS_API_KEY environment variable is not set or empty, skipping upload."
	exit 1
fi

# Build the gem
gem build metacall.gemspec

# Setup credentials for the push
# Note: RubyGems CLI expects the key in ~/.gem/credentials
mkdir -p ~/.gem
cat << EOF > ~/.gem/credentials
---
:rubygems_api_key: ${RUBYGEMS_API_KEY}
EOF
chmod 0600 ~/.gem/credentials

# Push the gem
gem push "metacall-${PORT_VERSION}.gem"

# Cleanup credentials and build artifacts
rm ~/.gem/credentials
rm "metacall-${PORT_VERSION}.gem"
