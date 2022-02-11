#!/usr/bin/env sh

#
#	MetaCall Build Bash Script by Parra Studios
#	Build and install bash script utility for MetaCall.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Enable BuildKit whenever possible
export DOCKER_BUILDKIT=1
export BUILDKIT_PROGRESS=plain

# Pull MetaCall Docker Compose
sub_pull() {
	if [ -z "$IMAGE_NAME" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	docker pull $IMAGE_NAME:deps && docker tag $IMAGE_NAME:deps metacall/core:deps || true

	docker pull $IMAGE_NAME:dev && docker tag $IMAGE_NAME:dev metacall/core:dev || true

	docker pull $IMAGE_NAME:runtime && docker tag $IMAGE_NAME:runtime metacall/core:runtime || true

	docker pull $IMAGE_NAME:cli && docker tag $IMAGE_NAME:cli metacall/core:cli || true
}

# Build MetaCall Docker Compose (link manually dockerignore files)
sub_build() {
	ln -sf tools/deps/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm cli
}

# Build MetaCall Docker Compose without cache (link manually dockerignore files)
sub_rebuild() {
	ln -sf tools/deps/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm --no-cache deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm --no-cache dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm --no-cache runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml build --force-rm --no-cache cli
}

# Build MetaCall Docker Compose with Sanitizer for testing (link manually dockerignore files)
sub_test() {

	# In order to get rid of the log limit:
	# [output clipped, log limit 1MiB reached]
	# Use this command:
	# docker buildx create --use --name larger_log --driver-opt env.BUILDKIT_STEP_LOG_MAX_SIZE=50000000

	ln -sf tools/deps/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.test.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.test.yml build --force-rm dev
}

# Build MetaCall Docker Compose with caching (link manually dockerignore files)
sub_cache() {
	if [ -z "$IMAGE_REGISTRY" ]; then
		echo "Error: IMAGE_REGISTRY variable not defined"
		exit 1
	fi

	ln -sf tools/deps/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.cache.yml build deps

	ln -sf tools/dev/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.cache.yml build dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.cache.yml build runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	docker-compose -f docker-compose.yml -f docker-compose.cache.yml build cli
}

# Push MetaCall Docker Compose
sub_push(){
	if [ -z "$IMAGE_NAME" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	# Push deps image
	docker tag metacall/core:deps $IMAGE_NAME:deps
	docker push $IMAGE_NAME:deps

	# Push dev image
	docker tag metacall/core:dev $IMAGE_NAME:dev
	docker push $IMAGE_NAME:dev

	# Push runtime image
	docker tag metacall/core:runtime $IMAGE_NAME:runtime
	docker push $IMAGE_NAME:runtime

	# Push cli image
	docker tag metacall/core:cli $IMAGE_NAME:cli
	docker push $IMAGE_NAME:cli

	# Push cli as a latest
	docker tag metacall/core:cli $IMAGE_NAME:latest
	docker push $IMAGE_NAME:latest
}

# Pack MetaCall Docker Compose
sub_pack(){
	if [ -z "$ARTIFACTS_PATH" ]; then
		echo "Error: ARTIFACTS_PATH variable not defined"
		exit 1
	fi

	# Get path where docker-compose.sh is located
	BASE_DIR="$(cd "$(dirname "$0")" && pwd)"

	# Load default environment variables
	. $BASE_DIR/.env

	# Run the package builds
	docker run --name metacall_core_pack -i metacall/core:dev /bin/bash -c 'cd build && make pack'

	# Create artifacts folder
	mkdir -p $ARTIFACTS_PATH

	# Copy artifacts
	docker cp metacall_core_pack:$METACALL_PATH/build/packages $ARTIFACTS_PATH

	# Remove docker instance
	docker rm metacall_core_pack

	# List generated artifacts
	ls -la $ARTIFACTS_PATH/packages
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` option"
	echo "Options:"
	echo "	pull"
	echo "	build"
	echo "	rebuild"
	echo "	test"
	echo "	cache"
	echo "	push"
	echo "	pack"
	echo ""
}

case "$1" in
	pull)
		sub_pull
		;;
	build)
		sub_build
		;;
	rebuild)
		sub_rebuild
		;;
	test)
		sub_test
		;;
	cache)
		sub_cache
		;;
	push)
		sub_push
		;;
	pack)
		sub_pack
		;;
	*)
		sub_help
		;;
esac
