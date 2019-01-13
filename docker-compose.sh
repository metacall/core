#!/bin/sh

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

# Pull MetaCall Docker Compose
sub_pull() {
	if [ -z "$IMAGE_NAME" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	docker pull $IMAGE_NAME:deps || true

	docker pull $IMAGE_NAME:dev || true

	docker pull $IMAGE_NAME:latest || true
}

# Build MetaCall Docker Compose (link manually dockerignore files)
sub_build() {
	ln -sf tools/base/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm dev

	ln -sf tools/core/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm core
}

# Build MetaCall Docker Compose for GitLab (link manually dockerignore files)
sub_build_gitlab() {
	ln -sf tools/base/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.gitlab-ci.yml build deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.gitlab-ci.yml build dev

	ln -sf tools/core/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.gitlab-ci.yml build core
}

# Push MetaCall Docker Compose
sub_push(){
	if [ -z "$IMAGE_NAME" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	docker tag metacall/core_deps:latest $IMAGE_NAME:deps
	docker push $IMAGE_NAME:deps

	docker tag metacall/core_dev:latest $IMAGE_NAME:dev
	docker push $IMAGE_NAME:dev

	docker tag metacall/core:latest $IMAGE_NAME:latest
	docker push $IMAGE_NAME:latest
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` option"
	echo "Options:"
	echo "	pull"
	echo "	build"
	echo "	build-gitlab"
	echo "	push"
	echo ""
}

case "$1" in
	pull)
		sub_pull
		;;
	build)
		sub_build
		;;
	build-gitlab)
		sub_build_gitlab
		;;
	push)
		sub_push
		;;
	*)
		sub_help
		;;
esac
