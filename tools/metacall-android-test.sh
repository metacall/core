#!/usr/bin/env bash

#
#	MetaCall Android Test Runner by Parra Studios
#	Wrapper script for running MetaCall tests on Android emulator via adb.
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

set -e

# Configuration
ANDROID_TEST_DIR="/data/local/tmp/metacall"
BUILD_DIR="${BUILD_DIR:-build}"
VERBOSE="${VERBOSE:-0}"
TEST_FILTER="${TEST_FILTER:-}"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a}"

# Known failing tests on Android with reasons
# These tests have known issues on Android and are skipped by default
# - detour-test: PLTHook library doesn't support Android (plthook_open fails)
# - metacall-fork-test: Depends on detour + fork() has Android Zygote limitations
# - configuration-test: Uses hardcoded host build paths in test config files
# - metacall-dynlink-path-test: Compares against hardcoded host build paths
# - environment-test: Requires specific test env vars (ENVIRONMENT_TEST_TEXT=abcd)
# Reference: https://github.com/kubo/plthook/issues/9
# Reference: https://source.android.com/docs/core/runtime/zygote
KNOWN_FAILING_TESTS="detour-test metacall-fork-test configuration-test metacall-dynlink-path-test environment-test"
SKIP_KNOWN_FAILING="${SKIP_KNOWN_FAILING:-1}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# Arrays for results
declare -a FAILED_TEST_NAMES=()
declare -a PASSED_TEST_NAMES=()
declare -a SKIPPED_TEST_NAMES=()

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_skip() {
    echo -e "${YELLOW}[SKIP]${NC} $1"
}

log_verbose() {
    if [ "$VERBOSE" = "1" ]; then
        echo -e "${BLUE}[DEBUG]${NC} $1"
    fi
}

show_help() {
    cat << EOF
MetaCall Android Test Runner

Usage: $(basename "$0") [OPTIONS]

Options:
    -b, --build-dir DIR     Path to build directory (default: build)
    -a, --abi ABI           Android ABI (arm64-v8a, armeabi-v7a, x86_64, x86)
    -f, --filter PATTERN    Run only tests matching pattern
    -v, --verbose           Enable verbose output
    -c, --clean             Clean test directory on device before pushing
    -l, --list              List available tests without running
    -k, --run-known-failing Run tests known to fail on Android
    -h, --help              Show this help message

Environment Variables:
    BUILD_DIR               Same as --build-dir
    ANDROID_ABI             Same as --abi (default: arm64-v8a)
    VERBOSE                 Set to 1 for verbose output
    TEST_FILTER             Same as --filter
    ANDROID_NDK_HOME        Path to Android NDK (for libc++_shared.so)
    SKIP_KNOWN_FAILING      Set to 0 to run known failing tests

Known Failing Tests (Android-specific issues):
    - detour-test           PLTHook not supported on Android
    - metacall-fork-test    Depends on detour + Android fork() limitations
    - configuration-test    Uses hardcoded host paths
    - metacall-dynlink-path-test  Uses hardcoded host paths
    - environment-test      Needs specific test env vars

Examples:
    # Run all tests (skipping known failures)
    $(basename "$0") -b build-android -c

    # Run only dynlink tests
    $(basename "$0") -b build-android -f dynlink

    # Run with verbose output
    $(basename "$0") -b build-android -v

    # Include known failing tests
    $(basename "$0") -b build-android -k

    # List available tests
    $(basename "$0") -b build-android -l
EOF
}

check_adb() {
    if ! command -v adb &> /dev/null; then
        log_error "adb not found. Please install Android SDK platform-tools."
        exit 1
    fi

    # Check if device is connected
    if ! adb get-state &> /dev/null; then
        log_error "No Android device/emulator connected. Please start an emulator or connect a device."
        exit 1
    fi

    log_info "ADB device connected: $(adb get-serialno)"
}

check_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        log_error "Build directory '$BUILD_DIR' not found."
        log_info "Please build MetaCall for Android first:"
        echo "  mkdir build-android && cd build-android"
        echo "  cmake .. -DCMAKE_TOOLCHAIN_FILE=\$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \\"
        echo "           -DANDROID_ABI=$ANDROID_ABI -DANDROID_PLATFORM=android-33 \\"
        echo "           -DOPTION_BUILD_TESTS=ON -DOPTION_BUILD_LOADERS=ON"
        echo "  cmake --build ."
        exit 1
    fi
}

# Check if a test is in the known failing list
is_known_failing() {
    local test_name="$1"
    for known in $KNOWN_FAILING_TESTS; do
        if [ "$test_name" = "$known" ]; then
            return 0
        fi
    done
    return 1
}

find_test_executables() {
    # Find all test executables (files named *-test or *_test without extension)
    find "$BUILD_DIR" -type f \( -name "*-test" -o -name "*_test" \) ! -name "*.so" ! -name "*.a" 2>/dev/null | sort
}

find_shared_libraries() {
    find "$BUILD_DIR" -name "*.so" -type f 2>/dev/null | sort
}

find_configuration_files() {
    find "$BUILD_DIR" -name "*.json" -path "*/configurations/*" -type f 2>/dev/null | sort
}

find_script_files() {
    find "$BUILD_DIR" -path "*/scripts/*" -type f 2>/dev/null | sort
}

clean_device() {
    log_info "Cleaning test directory on device..."
    adb shell "rm -rf $ANDROID_TEST_DIR" 2>/dev/null || true
    adb shell "mkdir -p $ANDROID_TEST_DIR"
    adb shell "mkdir -p $ANDROID_TEST_DIR/configurations"
    adb shell "mkdir -p $ANDROID_TEST_DIR/scripts"
}

# Get the correct ABI triple for NDK paths
get_abi_triple() {
    case "$ANDROID_ABI" in
        arm64-v8a)    echo "aarch64-linux-android" ;;
        armeabi-v7a)  echo "arm-linux-androideabi" ;;
        x86_64)       echo "x86_64-linux-android" ;;
        x86)          echo "i686-linux-android" ;;
        *)            echo "aarch64-linux-android" ;;
    esac
}

push_files_to_device() {
    log_info "Pushing files to Android device..."

    # Create directories
    adb shell "mkdir -p $ANDROID_TEST_DIR"
    adb shell "mkdir -p $ANDROID_TEST_DIR/configurations"
    adb shell "mkdir -p $ANDROID_TEST_DIR/scripts"

    # Push shared libraries
    local libs=$(find_shared_libraries)
    if [ -n "$libs" ]; then
        local lib_count=$(echo "$libs" | wc -l | tr -d ' ')
        log_info "Pushing $lib_count shared libraries..."
        echo "$libs" | while IFS= read -r lib; do
            [ -z "$lib" ] && continue
            log_verbose "Pushing $(basename "$lib")"
            adb push "$lib" "$ANDROID_TEST_DIR/" > /dev/null 2>&1
        done
    fi

    # Push libc++_shared.so from NDK if available
    # Reference: https://developer.android.com/ndk/guides/cpp-support
    if [ -n "${ANDROID_NDK_HOME:-}" ]; then
        # Detect host OS for prebuilt path
        # Note: darwin-x86_64 contains fat binaries that work on Apple Silicon too
        local host_tag
        case "$(uname -s)" in
            Linux*)  host_tag="linux-x86_64" ;;
            Darwin*) host_tag="darwin-x86_64" ;;
            MINGW*|MSYS*|CYGWIN*) host_tag="windows-x86_64" ;;
            *)       host_tag="linux-x86_64" ;;
        esac

        # Get correct ABI triple for the target architecture
        local abi_triple=$(get_abi_triple)
        local libcpp="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$host_tag/sysroot/usr/lib/$abi_triple/libc++_shared.so"

        if [ -f "$libcpp" ]; then
            log_info "Pushing libc++_shared.so from NDK ($abi_triple)..."
            adb push "$libcpp" "$ANDROID_TEST_DIR/" > /dev/null 2>&1
        else
            log_warning "libc++_shared.so not found at: $libcpp"
        fi
    fi

    # Push test executables
    local tests=$(find_test_executables)
    if [ -n "$tests" ]; then
        local test_count=$(echo "$tests" | wc -l | tr -d ' ')
        log_info "Pushing $test_count test executables..."
        echo "$tests" | while IFS= read -r test; do
            [ -z "$test" ] && continue
            log_verbose "Pushing $(basename "$test")"
            adb push "$test" "$ANDROID_TEST_DIR/" > /dev/null 2>&1
        done
    fi

    # Push configuration files
    local configs=$(find_configuration_files)
    if [ -n "$configs" ]; then
        log_info "Pushing configuration files..."
        echo "$configs" | while IFS= read -r config; do
            [ -z "$config" ] && continue
            log_verbose "Pushing $(basename "$config")"
            adb push "$config" "$ANDROID_TEST_DIR/configurations/" > /dev/null 2>&1
        done
    fi

    # Push script files
    local scripts=$(find_script_files)
    if [ -n "$scripts" ]; then
        log_info "Pushing script files..."
        echo "$scripts" | while IFS= read -r script; do
            [ -z "$script" ] && continue
            log_verbose "Pushing $(basename "$script")"
            adb push "$script" "$ANDROID_TEST_DIR/scripts/" > /dev/null 2>&1
        done
    fi

    # Make test executables executable
    adb shell "chmod +x $ANDROID_TEST_DIR/*-test $ANDROID_TEST_DIR/*_test" 2>/dev/null || true

    log_info "Files pushed to device at $ANDROID_TEST_DIR"
}

list_tests() {
    log_info "Available tests:"
    local tests=$(find_test_executables)
    if [ -n "$tests" ]; then
        echo "$tests" | while IFS= read -r test; do
            [ -z "$test" ] && continue
            local test_name=$(basename "$test")
            if [ -z "$TEST_FILTER" ] || echo "$test_name" | grep -q "$TEST_FILTER"; then
                if is_known_failing "$test_name"; then
                    echo -e "  ${YELLOW}$test_name${NC} (known failing)"
                else
                    echo "  $test_name"
                fi
            fi
        done
    else
        log_warning "No tests found."
    fi
}

run_single_test() {
    local test_name="$1"

    # Build environment variables for MetaCall
    local env_vars="LD_LIBRARY_PATH=$ANDROID_TEST_DIR"
    env_vars="$env_vars LOADER_LIBRARY_PATH=$ANDROID_TEST_DIR"
    env_vars="$env_vars LOADER_SCRIPT_PATH=$ANDROID_TEST_DIR/scripts"
    env_vars="$env_vars CONFIGURATION_PATH=$ANDROID_TEST_DIR/configurations/global.json"
    env_vars="$env_vars SERIAL_LIBRARY_PATH=$ANDROID_TEST_DIR"
    env_vars="$env_vars DETOUR_LIBRARY_PATH=$ANDROID_TEST_DIR"
    env_vars="$env_vars DYNLINK_TEST_LIBRARY_PATH=$ANDROID_TEST_DIR"

    # Run test without timeout (Android toybox timeout has compatibility issues)
    # Reference: https://github.com/jackpal/Android-Terminal-Emulator/wiki/Android-Shell-Command-Reference
    local start_time=$(date +%s)
    local exit_code

    # Execute the test and capture exit code
    exit_code=$(adb shell "cd $ANDROID_TEST_DIR && $env_vars ./$test_name >/dev/null 2>&1; echo \$?" 2>&1 | tr -d '\r\n')

    local end_time=$(date +%s)
    local duration=$((end_time - start_time))

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    # Check result
    if [ "$exit_code" = "0" ]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        PASSED_TEST_NAMES+=("$test_name")
        log_success "$test_name (${duration}s)"
        return 0
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_TEST_NAMES+=("$test_name")
        log_error "$test_name (exit code: $exit_code, ${duration}s)"

        # Show output for failed tests in verbose mode
        if [ "$VERBOSE" = "1" ]; then
            echo "--- Test Output ---"
            adb shell "cd $ANDROID_TEST_DIR && $env_vars ./$test_name 2>&1" | tail -30
            echo "--- End Output ---"
        fi
        return 1
    fi
}

run_all_tests() {
    log_info "Starting test execution..."
    echo ""

    # Get list of test names on device using IFS to handle newlines properly
    # Reference: https://www.baeldung.com/linux/variable-preserve-linebreaks
    local OLD_IFS="$IFS"
    IFS=$'\n'

    local device_tests_raw=$(adb shell "cd $ANDROID_TEST_DIR && ls *-test *_test 2>/dev/null" | tr -d '\r')
    local device_tests=($device_tests_raw)

    IFS="$OLD_IFS"

    if [ ${#device_tests[@]} -eq 0 ]; then
        log_error "No test executables found on device. Did you run with --clean first?"
        return 1
    fi

    log_info "Found ${#device_tests[@]} tests on device"
    echo ""

    # Run each test
    for test_name in "${device_tests[@]}"; do
        # Skip empty entries
        [ -z "$test_name" ] && continue

        # Skip configuration-default-test directory marker
        [[ "$test_name" == *":"* ]] && continue

        # Apply filter if set
        if [ -n "$TEST_FILTER" ] && ! echo "$test_name" | grep -q "$TEST_FILTER"; then
            continue
        fi

        # Check if test is known to fail on Android
        if [ "$SKIP_KNOWN_FAILING" = "1" ] && is_known_failing "$test_name"; then
            SKIPPED_TESTS=$((SKIPPED_TESTS + 1))
            SKIPPED_TEST_NAMES+=("$test_name")
            log_skip "$test_name (known Android issue)"
            continue
        fi

        log_info "Running: $test_name"
        run_single_test "$test_name" || true
    done
}

print_summary() {
    echo ""
    echo "=========================================="
    echo "       ANDROID TEST RESULTS SUMMARY       "
    echo "=========================================="
    echo ""
    echo -e "Total:   ${BLUE}$TOTAL_TESTS${NC}"
    echo -e "Passed:  ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Failed:  ${RED}$FAILED_TESTS${NC}"
    if [ "$SKIPPED_TESTS" -gt 0 ]; then
        echo -e "Skipped: ${YELLOW}$SKIPPED_TESTS${NC} (known Android issues)"
    fi
    echo ""

    if [ ${#SKIPPED_TEST_NAMES[@]} -gt 0 ]; then
        echo -e "${YELLOW}Skipped tests (known Android issues):${NC}"
        for test_name in "${SKIPPED_TEST_NAMES[@]}"; do
            echo "  - $test_name"
        done
        echo ""
    fi

    if [ "$FAILED_TESTS" -gt 0 ]; then
        echo -e "${RED}Failed tests:${NC}"
        for test_name in "${FAILED_TEST_NAMES[@]}"; do
            echo "  - $test_name"
        done
        echo ""
    fi

    # Calculate pass rate
    if [ "$TOTAL_TESTS" -gt 0 ]; then
        local pass_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
        echo -e "Pass rate: ${GREEN}${pass_rate}%${NC}"
        echo ""
    fi

    if [ "$FAILED_TESTS" -eq 0 ] && [ "$TOTAL_TESTS" -gt 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    elif [ "$TOTAL_TESTS" -eq 0 ]; then
        echo -e "${YELLOW}No tests were run.${NC}"
        return 1
    else
        echo -e "${RED}Some tests failed.${NC}"
        return 1
    fi
}

# Parse command line arguments
CLEAN_DEVICE=0
LIST_ONLY=0

while [ $# -gt 0 ]; do
    case "$1" in
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -a|--abi)
            ANDROID_ABI="$2"
            shift 2
            ;;
        -f|--filter)
            TEST_FILTER="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -c|--clean)
            CLEAN_DEVICE=1
            shift
            ;;
        -l|--list)
            LIST_ONLY=1
            shift
            ;;
        -k|--run-known-failing)
            SKIP_KNOWN_FAILING=0
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Main execution
main() {
    echo ""
    echo "=========================================="
    echo "    MetaCall Android Test Runner         "
    echo "=========================================="
    echo ""

    check_adb
    check_build_dir

    log_info "Build directory: $BUILD_DIR"
    log_info "Target ABI: $ANDROID_ABI"

    if [ "$LIST_ONLY" = "1" ]; then
        list_tests
        exit 0
    fi

    if [ "$CLEAN_DEVICE" = "1" ]; then
        clean_device
    fi

    push_files_to_device
    echo ""

    run_all_tests

    print_summary
}

main
