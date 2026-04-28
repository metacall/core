#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./python-valgrind.sh <python-src-dir>
# Example:
#   ./python-valgrind.sh ~/build/python3.13-*/

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <python-src-dir>"
  exit 1
fi

PY_SRC="$1"

pushd "$PY_SRC" >/dev/null

./configure \
  --with-pydebug \
  --without-pymalloc \
  --with-valgrind \
  --with-ensurepip=no

make -j"$(nproc)"
make altinstall

popd >/dev/null

echo "Python build with Valgrind support completed."
