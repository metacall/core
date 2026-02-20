#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="${SCRIPT_DIR}/.ghc-build"

os_name=$(uname -s)
case "${os_name}" in
	Darwin)
		default_output="${SCRIPT_DIR}/metacall_hs_bridge.dylib"
		is_windows=0
		;;
	MINGW*|MSYS*|CYGWIN*)
		default_output="${SCRIPT_DIR}/metacall_hs_bridge.dll"
		is_windows=1
		;;
	*)
		default_output="${SCRIPT_DIR}/metacall_hs_bridge.so"
		is_windows=0
		;;
esac

output_path="${1:-${default_output}}"

mkdir -p "${BUILD_DIR}"

if [ "${is_windows}" -eq 1 ]; then
	ghc \
		--make \
		-shared \
		-no-hs-main \
		-outputdir "${BUILD_DIR}" \
		"${SCRIPT_DIR}/MetacallHSBridge.hs" \
		-optl-Wl,--export-all-symbols \
		-optl-Wl,--undefined=metacall_hs_module_initialize \
		-optl-Wl,--undefined=metacall_hs_module_destroy \
		-optl-Wl,--undefined=metacall_hs_module_function_count \
		-optl-Wl,--undefined=metacall_hs_module_function_name \
		-optl-Wl,--undefined=metacall_hs_module_function_args_count \
		-optl-Wl,--undefined=metacall_hs_module_function_arg_name \
		-optl-Wl,--undefined=metacall_hs_module_function_arg_type \
		-optl-Wl,--undefined=metacall_hs_module_function_return_type \
		-optl-Wl,--undefined=metacall_hs_module_invoke \
		-optl-Wl,--undefined=metacall_hs_module_value_destroy \
		-optl-Wl,--undefined=metacall_hs_module_last_error \
		-o "${output_path}"
else
	ghc \
		--make \
		-dynamic \
		-shared \
		-fPIC \
		-no-hs-main \
		-outputdir "${BUILD_DIR}" \
		"${SCRIPT_DIR}/MetacallHSBridge.hs" \
		-o "${output_path}"
fi

echo "Built: ${output_path}"
