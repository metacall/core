#!/usr/bin/env bash

#
#	MetaCall Rust Port Deploy Script by Parra Studios
#	Script utility for deploying MetaCall Rust Port to Crates.
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

function publish() {
    local crate_version=`cargo search --quiet $1 | grep "$1" | head -n 1 | awk '{ print $3 }'`
    local project_version=`cargo metadata --format-version=1 --no-deps | jq '.packages[0].version'`

    # Check if versions do not match, and if so, publish them
    if [ ! "${crate_version}" = "${project_version}" ]; then
        echo "Publishing $1: ${crate_version} -> ${project_version}"
        cargo publish --verbose --locked --token ${CARGO_REGISTRY_TOKEN}
    fi
}

# Publish
cd inline
publish metacall-inline
cd ..
publish metacall
