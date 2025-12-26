#!/usr/bin/env bash

#
#	MetaCall Build Bash Script by Parra Studios
#	Build and install bash script utility for MetaCall.
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

# List of tags
METACALL_TAGS=("deps" "dev" "runtime" "cli")

# Pull MetaCall Docker Compose
sub_pull() {
	if [ -z "${IMAGE_NAME+x}" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	for tag in "${METACALL_TAGS[@]}"; do
		docker pull $IMAGE_NAME:${tag} && docker tag $IMAGE_NAME:${tag} metacall/core:${tag} || true
	done
}

# Build MetaCall Docker Compose (link manually dockerignore files)
sub_build() {
	for tag in "${METACALL_TAGS[@]}"; do
		ln -sf tools/${tag}/.dockerignore .dockerignore
		$DOCKER_COMPOSE -f docker-compose.yml build --force-rm ${tag}
	done
}

# Build MetaCall Docker Compose without cache (link manually dockerignore files)
sub_rebuild() {
	for tag in "${METACALL_TAGS[@]}"; do
		ln -sf tools/${tag}/.dockerignore .dockerignore
		$DOCKER_COMPOSE -f docker-compose.yml build --force-rm --no-cache ${tag}
	done
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

		if [ -z "${BEGIN+x}" ] || [ -z "${END+x}" ]; then
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
	if [ -z "${IMAGE_REGISTRY+x}" ]; then
		echo "Error: IMAGE_REGISTRY variable not defined"
		exit 1
	fi

	for tag in "${METACALL_TAGS[@]}"; do
		ln -sf tools/${tag}/.dockerignore .dockerignore
		$DOCKER_COMPOSE -f docker-compose.yml -f docker-compose.cache.yml build ${tag}
	done
}

# Build MetaCall Docker Compose with multi-platform specifier (link manually dockerignore files)
sub_platform() {
	if [ -z "${METACALL_PLATFORM+x}" ]; then
		echo "Error: METACALL_PLATFORM variable not defined"
		exit 1
	fi

	# Initialize QEMU for Buildkit
	docker run --rm --privileged tonistiigi/binfmt --install all

	# Load, clear and export default environment variables
	export $(cat .env | sed 's/#.*//g' | xargs)

	# Debian in Docker Hub does not support LoongArch64 yet, let's use official LoongArch repository instead
	if [ "$METACALL_PLATFORM" = "linux/loong64" ]; then
		export METACALL_BASE_IMAGE="ghcr.io/loong64/${METACALL_BASE_IMAGE}"
	fi

	# Generate the docker compose file with all .env variables substituted (bake seems not to support this)
	$DOCKER_COMPOSE -f docker-compose.yml config &> docker-compose.bake.yml

	# Build with Bake, so the image can be loaded into local docker context
	for tag in "${METACALL_TAGS[@]}"; do
		ln -sf "tools/${tag}/.dockerignore" .dockerignore
		docker buildx bake -f docker-compose.bake.yml --set *.platform="${METACALL_PLATFORM}" --load "${tag}"
	done

	# Delete temporal docker compose file
	rm -rf docker-compose.bake.yml
}

# Build MetaCall Docker Compose with multi-platform specifier
sub_bake() {
	if [ -z "${METACALL_PLATFORM+x}" ]; then
		echo "Error: METACALL_PLATFORM variable not defined"
		exit 1
	fi

	if [ -z "${DOCKER_USERNAME+x}" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	if [ -z "${IMAGE_NAME+x}" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	# Initialize QEMU for Buildkit
	docker run --rm --privileged tonistiigi/binfmt --install all

	# Load, clear and export default environment variables
	export $(cat .env | sed 's/#.*//g' | xargs)

	# Get the options from the compose file
	export METACALL_INSTALL_OPTIONS=$(grep "METACALL_INSTALL_OPTIONS:" docker-compose.yml | head -n 1 | sed 's/.*METACALL_INSTALL_OPTIONS: //' | sed 's/#.*//g')
	export METACALL_BUILD_OPTIONS=$(grep "METACALL_BUILD_OPTIONS:" docker-compose.yml | head -n 1 | sed 's/.*METACALL_BUILD_OPTIONS: //' | sed 's/#.*//g')
	export METACALL_RUNTIME_OPTIONS=$(grep "METACALL_RUNTIME_OPTIONS:" docker-compose.yml | head -n 1 | sed 's/.*METACALL_RUNTIME_OPTIONS: //' | sed 's/#.*//g')

	# Debian in Docker Hub does not support LoongArch64 yet, let's use official LoongArch repository instead
	if [ "$METACALL_PLATFORM" = "linux/loong64" ]; then
		export METACALL_BASE_IMAGE="ghcr.io/loong64/${METACALL_BASE_IMAGE}"
	fi

	# Create temporal folder for storing metadata
	mkdir -p .bake

	# Generate the dockerignore file by merging all of them
	echo "**" > .bake/.dockerignore
	for f in tools/deps/.dockerignore tools/dev/.dockerignore tools/runtime/.dockerignore tools/cli/.dockerignore; do
		tail -n +2 "$f" >> .bake/.dockerignore
	done
	ln -sf .bake/.dockerignore .dockerignore

	# Build all images all at once
	docker buildx bake \
		--metadata-file .bake/metadata.json \
		-f docker-bake.hcl \
		--set "*.platform=${METACALL_PLATFORM}"

	cat .bake/metadata.json
}

# Push MetaCall Docker Compose
sub_push() {
	if [ -z "${IMAGE_NAME+x}" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	# Push images
	for tag in "${METACALL_TAGS[@]}"; do
		docker tag metacall/core:${tag} $IMAGE_NAME:${tag}
		docker push $IMAGE_NAME:${tag}
	done

	# Push cli as a latest
	docker tag metacall/core:cli $IMAGE_NAME:latest
	docker push $IMAGE_NAME:latest
}

# Version MetaCall Docker Compose
sub_version() {
	if [ -z "${IMAGE_NAME+x}" ]; then
		echo "Error: IMAGE_NAME variable not defined"
		exit 1
	fi

	VERSION=$(tail -n 1 VERSION | tr -d '\n')

	# Push images
	for tag in "${METACALL_TAGS[@]}"; do
		docker tag metacall/core:${tag} $IMAGE_NAME:${VERSION}-${tag}
		docker push $IMAGE_NAME:${VERSION}-${tag}
	done

	# Push cli image as version
	docker tag metacall/core:cli $IMAGE_NAME:${VERSION}
	docker push $IMAGE_NAME:${VERSION}
}

# Pack MetaCall Docker Compose
sub_pack() {
	if [ -z "${ARTIFACTS_PATH+x}" ]; then
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
	bake)
		sub_bake
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
