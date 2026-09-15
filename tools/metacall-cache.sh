#!/usr/bin/env sh

#
#	MetaCall Build Shell Script by Parra Studios
#	Compiler cache utility: identity, environment and statistics.
#

set -eu

SCCACHE_VERSION_PINNED="${SCCACHE_VERSION:-0.18.0}"
CACHE_ROOT="${METACALL_CACHE_ROOT:-${HOME}/.cache/metacall}"
CACHE_SIZE="${METACALL_CACHE_SIZE:-2G}"

sub_backend() {
	case "${METACALL_CACHE_BACKEND:-}" in
	gha) printf 'gha\n' ;;
	registry) printf 'registry\n' ;;
	local | "") printf 'local\n' ;;
	*)
		echo "unsupported METACALL_CACHE_BACKEND '${METACALL_CACHE_BACKEND}', use gha, registry or local" >&2
		exit 1
		;;
	esac
}

# Profile separates instrumentations so each one has its own budget and hit rate.
sub_profile() {
	asan=""
	tsan=""
	msan=""
	memcheck=""
	coverage=""

	for option in "$@"; do
		case "$option" in
		address-sanitizer) asan=1 ;;
		thread-sanitizer) tsan=1 ;;
		memory-sanitizer) msan=1 ;;
		memcheck) memcheck=1 ;;
		coverage) coverage=1 ;;
		esac
	done

	# Same precedence as cmake/Cache.cmake, a sanitizer decides the flags of the objects
	if [ -n "${METACALL_CACHE_PROFILE:-}" ]; then
		printf '%s\n' "${METACALL_CACHE_PROFILE}"
	elif [ -n "${asan}" ]; then
		printf 'asan\n'
	elif [ -n "${tsan}" ]; then
		printf 'tsan\n'
	elif [ -n "${msan}" ]; then
		printf 'msan\n'
	elif [ -n "${memcheck}" ]; then
		printf 'memcheck\n'
	elif [ -n "${coverage}" ]; then
		printf 'coverage\n'
	else
		sub_build_type "$@"
	fi
}

sub_build_type() {
	for option in "$@"; do
		case "$option" in
		debug | release | relwithdebinfo)
			printf '%s\n' "${option}"
			return 0
			;;
		esac
	done

	printf '%s\n' "${METACALL_BUILD_TYPE:-debug}"
}

sub_compiler() {
	for option in "$@"; do
		case "$option" in
		clang | clang-msan)
			printf '%s\n' "clang"
			return 0
			;;
		esac
	done

	case "${CC:-}" in
	*clang*)
		printf '%s\n' "clang"
		return 0
		;;
	esac

	printf '%s\n' "gcc"
}

# Cache identity must be stable across runners and safe as a path and as a cache key.
sub_id() {
	base_image="${METACALL_BASE_IMAGE:-unknown}"
	build_type="$(sub_build_type "$@")"
	profile="$(sub_profile "$@")"
	id="metacall-$(sub_backend)-${base_image}-$(sub_compiler "$@")-${build_type}"

	if [ "${profile}" != "${build_type}" ]; then
		id="${id}-${profile}"
	fi

	printf '%s\n' "${id}" | tr '[:upper:]' '[:lower:]' | tr ':/+. ' '-----'
}

sub_dir() {
	printf '%s\n' "${METACALL_CACHE_DIR:-${CACHE_ROOT}/sccache/$(sub_id "$@")}"
}

sub_env() {
	id="$(sub_id "$@")"
	dir="$(sub_dir "$@")"

	printf '%s\n' "METACALL_CACHE_BACKEND=$(sub_backend)"
	printf '%s\n' "METACALL_CACHE_PROFILE=$(sub_profile "$@")"
	printf '%s\n' "METACALL_CACHE_ID=${id}"
	printf '%s\n' "METACALL_CACHE_KEY=${id}"
	printf '%s\n' "METACALL_CACHE_DIR=${dir}"
	printf '%s\n' "SCCACHE_DIR=${dir}"
	printf '%s\n' "SCCACHE_CACHE_SIZE=${CACHE_SIZE}"
	printf '%s\n' "SCCACHE_BASEDIRS=${METACALL_PATH:-$(pwd)}"

	# gha moves this directory, sccache's own GHA backend stays unused: GHAC rejects small object writes
}

sub_stats() {
	if ! command -v sccache >/dev/null 2>&1; then
		echo "sccache is not installed" >&2
		return 1
	fi

	case "${1:-}" in
	--zero) sccache --zero-stats ;;
	--json) sccache --show-stats --stats-format=json ;;
	--summary) sccache --show-stats | grep -E 'Compile requests|Cache hits|Cache misses|Hit rate' || true ;;
	*) sccache --show-stats ;;
	esac
}

# The server keeps the cache directory of its first client, so a profile switch needs a restart.
sub_restart() {
	sccache --stop-server >/dev/null 2>&1 || true
	sub_env "$@"
}

sub_help() {
	echo "Usage: $(basename "$0") option [build options]"
	echo "	env: print the cache environment of the selected profile"
	echo "	id | key | dir | profile: print one identity value"
	echo "	stats [--zero|--json|--summary]: sccache statistics"
	echo "	restart: stop the sccache server and print the environment"
	echo "	version: print the pinned sccache version"
	echo "	self-test: verify the identity function"
	echo "	backends: local disk when METACALL_CACHE_BACKEND is local or empty, gha for the Actions cache, registry for a cache image repository"
	echo "	help: print this help"
	echo ""
}

sub_self_test() {
	expect() {
		if [ "$2" != "$3" ]; then
			echo "FAIL ${1}: expected ${3}, got ${2}" >&2
			exit 1
		fi
	}

	expect "profile asan" "$(sub_profile debug address-sanitizer tests)" "asan"
	expect "profile asan wins over memcheck" "$(sub_profile debug memcheck address-sanitizer)" "asan"
	expect "profile asan wins over tsan" "$(sub_profile debug thread-sanitizer address-sanitizer)" "asan"
	expect "profile memcheck wins over coverage" "$(sub_profile debug coverage memcheck)" "memcheck"
	expect "profile override" "$(METACALL_CACHE_PROFILE=asan sub_profile debug)" "asan"
	expect "profile memcheck" "$(sub_profile debug memcheck)" "memcheck"
	expect "profile default" "$(sub_profile release python)" "release"
	expect "compiler clang" "$(sub_compiler debug clang-msan)" "clang"
	expect "compiler gcc" "$( (unset CC CXX; sub_compiler debug memcheck) )" "gcc"
	expect "build type" "$(sub_build_type debug python)" "debug"
	expect "id" "$(METACALL_BASE_IMAGE='debian:trixie-slim' sub_id debug memcheck)" \
		"metacall-local-debian-trixie-slim-gcc-debug-memcheck"
	expect "id without instrumentation" "$(METACALL_BASE_IMAGE='ubuntu:noble' sub_id release)" \
		"metacall-local-ubuntu-noble-gcc-release"
	expect "backend gha" "$(METACALL_BASE_IMAGE='ubuntu:noble' METACALL_CACHE_BACKEND=gha sub_id debug address-sanitizer)" \
		"metacall-gha-ubuntu-noble-gcc-debug-asan"
	expect "backend local" "$(METACALL_BASE_IMAGE='ubuntu:noble' METACALL_CACHE_BACKEND=local sub_id release)" \
		"metacall-local-ubuntu-noble-gcc-release"

	echo "self-test: ok"
}

# Stop before any output when the backend is unknown
sub_backend >/dev/null

case "${1:-help}" in
env)
	shift
	sub_env "$@"
	;;
id)
	shift
	sub_id "$@"
	;;
key)
	shift
	sub_id "$@"
	;;
dir)
	shift
	sub_dir "$@"
	;;
profile)
	shift
	sub_profile "$@"
	;;
stats)
	shift
	sub_stats "$@"
	;;
restart)
	shift
	sub_restart "$@"
	;;
version) printf '%s\n' "${SCCACHE_VERSION_PINNED}" ;;
self-test) sub_self_test ;;
help | *) sub_help ;;
esac
