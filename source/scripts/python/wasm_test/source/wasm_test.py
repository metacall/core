#!/usr/bin/env python3

from metacall import metacall, metacall_load_from_file

import empty_module.wat
import empty_module.wasm

# We can't import both functions.wasm and functions.wat here because their
# definitions would collide.
from functions.wasm import *

assert none_ret_none() is None
assert i32_ret_none(0) is None
assert i32_f32_i64_f64_ret_none(0) is None
assert none_ret_i32() == 1
assert none_ret_i32_f32_i64_f64() == [1, 2, 3, 4]
assert i32_f32_i64_f64_ret_i32_f32_i64_f64(0, 0, 0, 0) == [1, 2, 3, 4]
assert trap() is None

# We test an invalid load file attempt first to avoid polluting the global
# handle for the next test.
assert not metacall_load_from_file("wasm", ["exports1.wat", "imports.wat"])

assert metacall_load_from_file(
    "wasm", ["exports1.wat", "exports2.wat", "imports.wat"]
)
assert metacall("duplicate_func_i32") == 1
assert metacall("duplicate_func_i64") == 2

for module in ("invalid_module.wat", "invalid_module.wasm"):
    try:
        __import__(module)
    except ImportError:
        pass
    else:
        raise AssertionError("Importing an invalid module should result in an ImportError")
