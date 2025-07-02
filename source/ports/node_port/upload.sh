#!/usr/bin/env bash

#
#	MetaCall NodeJS Port Deploy Script by Parra Studios
#	Script utility for deploying MetaCall NodeJS Port to NPM.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

set -euxo pipefail

NPM_VERSION=$(npm view metacall version)
PORT_VERSION=$(node -p "require('./package.json').version")

if [[ "$NPM_VERSION" == "$PORT_VERSION" ]]; then
	echo "Current package version is the same as NPM version, skipping upload."
	exit 0
fi

if [[ -z "${NPM_TOKEN:-}" ]]; then
	echo "NPM_TOKEN environment variable is not set or empty, skipping upload"
	exit 1
fi

# Register the token
echo "//registry.npmjs.org/:_authToken=${NPM_TOKEN}" > ~/.npmrc

# Publish
npm login
npm publish
