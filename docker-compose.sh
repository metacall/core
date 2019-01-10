#!/bin/bash

#
#	MetaCall Build Bash Script by Parra Studios
#	Build and install bash script utility for MetaCall.
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

# Build MetaCall Docker Compose (link manually dockerignore files)
sub_build() {
	ln -sf tools/base/.dockerignore .dockerignore
	docker-compose build --force-rm deps

	ln -sf tools/node/.dockerignore .dockerignore
	docker-compose build --force-rm deps_node

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose build --force-rm dev

	ln -sf tools/core/.dockerignore .dockerignore
	docker-compose build --force-rm core
}

# Push MetaCall Docker Compose
sub_push(){
	docker tag metacall/core:latest $IMAGE_NAME
	docker push $IMAGE_NAME
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` option"
	echo "Options:"
	echo "	build"
	echo ""
}

case "$1" in
	build)
		sub_build
		;;
	*)
		sub_help
		;;
esac
