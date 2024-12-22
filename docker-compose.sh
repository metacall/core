#!/usr/bin/env bash

#
#	MetaCall Build Bash Script by Parra Studios
#	Build and install bash script utility for MetaCall.
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

set -euxo pipefail

# Enable BuildKit whenever possible
export COMPOSE_DOCKER_CLI_BUILD=1
export DOCKER_BUILDKIT=1
export BUILDKIT_PROGRESS=plain
export PROGRESS_NO_TRUNC=1

# Check if docker compose command is available
if [ -x "$(command -v docker-compose)" ]; then
	DOCKER_COMPOSE="docker-compose"
elif $(docker compose &>/dev/null) && [ $? -eq 0 ]; then
	DOCKER_COMPOSE="docker compose"
else
	echo "Docker Compose not installed, install it and re-run the script"
	exit 1
fi

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
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm cli
}

# Build MetaCall Docker Compose without cache (link manually dockerignore files)
sub_rebuild() {
	ln -sf tools/deps/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm --no-cache deps

	ln -sf tools/dev/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm --no-cache dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm --no-cache runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml build --force-rm --no-cache cli
}

# Build MetaCall Docker Compose for testing (link manually dockerignore files)
sub_test() {
	# Disable BuildKit as workaround due to log limits (TODO: https://github.com/docker/buildx/issues/484)
	export DOCKER_BUILDKIT=0

	# Disable build with sanitizer
	export METACALL_BUILD_SANITIZER=

	# Disable build with coverage
	export METACALL_BUILD_COVERAGE=

	# Define build type
	export METACALL_BUILD_TYPE=${METACALL_BUILD_TYPE:-debug}

	ln -sf tools/deps/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm dev
}

# Build MetaCall Docker Compose with Sanitizer for testing (link manually dockerignore files)
sub_test_sanitizer() {
	# Disable BuildKit as workaround due to log limits (TODO: https://github.com/docker/buildx/issues/484)
	export DOCKER_BUILDKIT=0

	# Enable build with sanitizer
	export METACALL_BUILD_SANITIZER=${METACALL_BUILD_SANITIZER:-address-sanitizer}

	# Disable build with coverage
	export METACALL_BUILD_COVERAGE=

	# Define build type
	export METACALL_BUILD_TYPE=${METACALL_BUILD_TYPE:-debug}

	ln -sf tools/deps/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore

	if [ ! -z "${SANITIZER_SKIP_SUMMARY:-}" ]; then
		$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm dev
	else
		$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm dev | tee /tmp/metacall-test-output

		# Retrieve all the summaries
		SUMMARY=$(grep "SUMMARY:" /tmp/metacall-test-output)
		echo "${SUMMARY}"
		printf "Number of leaks detected: "
		echo "${SUMMARY}" | awk '{print $7}' | awk '{s+=$1} END {print s}'

		# Count the number of tests that really failed and avoid the false positives
		FAILED=$(grep "FAILED TEST" /tmp/metacall-test-output)
		printf "Number of tests failed: "
		echo "${FAILED}" | awk '{print $1}' | awk '{s+=$1} END {print s}'

		# Get the potential tests that failed
		BEGIN=$(grep -n "The following tests FAILED:" /tmp/metacall-test-output | cut -d : -f 1)
		END=$(grep -n "Errors while running CTest" /tmp/metacall-test-output | cut -d : -f 1)

		if [ -z "${BEGIN}" ] || [ -z "${END}" ]; then
			echo "ERROR! CTest failed to print properly the output, run tests again:"
			echo "	Recompiling everything: docker rmi metacall/core:dev && ./docker-compose.sh test-${METACALL_BUILD_SANITIZER}"
			echo "	Without recompiling (needs to be built successfully previously): docker run --rm -it metacall/core:dev sh -c \"cd build && ctest -j$(getconf _NPROCESSORS_ONLN) --output-on-failure\""
		else
			BEGIN=$((BEGIN + 1))
			END=$((END - 1))
			echo "List of potential failed tests:"
			sed -n "${BEGIN},${END}p" /tmp/metacall-test-output
		fi

		rm /tmp/metacall-test-output
	fi
}

# Build MetaCall Docker Compose for coverage (link manually dockerignore files)
sub_coverage() {
	# Disable BuildKit as workaround due to log limits (TODO: https://github.com/docker/buildx/issues/484)
	export DOCKER_BUILDKIT=0

	# Disable build with sanitizer
	export METACALL_BUILD_SANITIZER=

	# Disable build with coverage
	export METACALL_BUILD_COVERAGE=coverage

	# Define build type
	export METACALL_BUILD_TYPE=debug

	ln -sf tools/deps/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm deps

	ln -sf tools/dev/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.test.yml build --force-rm dev
}

# Build MetaCall Docker Compose with caching (link manually dockerignore files)
sub_cache() {
	if [ -z "$IMAGE_REGISTRY" ]; then
		echo "Error: IMAGE_REGISTRY variable not defined"
		exit 1
	fi

	ln -sf tools/deps/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.cache.yml build deps

	ln -sf tools/dev/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.cache.yml build dev

	ln -sf tools/runtime/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.cache.yml build runtime

	ln -sf tools/cli/.dockerignore .dockerignore
	$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.cache.yml build cli
}

sub_platform() {
    
}

# Push MetaCall Docker Compose
sub_push() {
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

# Version MetaCall Docker Compose
sub_version() {
	if [ -z "$IMAGE_NAME" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	VERSION=$(tail -n 1 VERSION | tr -d '\n')

	# Push deps image
	docker tag metacall/core:deps $IMAGE_NAME:${VERSION}-deps
	docker push $IMAGE_NAME:${VERSION}-deps

	# Push dev image
	docker tag metacall/core:dev $IMAGE_NAME:${VERSION}-dev
	docker push $IMAGE_NAME:${VERSION}-dev

	# Push runtime image
	docker tag metacall/core:runtime $IMAGE_NAME:${VERSION}-runtime
	docker push $IMAGE_NAME:${VERSION}-runtime

	# Push cli image
	docker tag metacall/core:cli $IMAGE_NAME:${VERSION}-cli
	docker push $IMAGE_NAME:${VERSION}-cli

	# Push cli image as version
	docker tag metacall/core:cli $IMAGE_NAME:${VERSION}
	docker push $IMAGE_NAME:${VERSION}
}

# Pack MetaCall Docker Compose
sub_pack() {
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
	echo "	test-address-sanitizer"
	echo "	test-thread-sanitizer"
	echo "	test-memory-sanitizer"
	echo "	coverage"
	echo "	cache"
	echo "	platform"
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
	test-address-sanitizer)
		export METACALL_BUILD_SANITIZER="address-sanitizer"
		sub_test_sanitizer
		;;
	test-thread-sanitizer)
		export METACALL_BUILD_SANITIZER="thread-sanitizer"
		sub_test_sanitizer
		;;
	test-memory-sanitizer)
		export METACALL_BUILD_SANITIZER="memory-sanitizer"
		sub_test_sanitizer
		;;
	coverage)
		sub_coverage
		;;
	cache)
		sub_cache
		;;
	platform)
		sub_platform
		;;
	push)
		sub_push
		;;
	version)
		sub_version
		;;
	pack)
		sub_pack
		;;
	*)
		sub_help
		;;
esac
