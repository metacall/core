# Lua Loader

### What is the Lua Loader

The Lua loader (`lua_loader`) embeds a LuaJIT virtual machine into MetaCall.
It loads Lua scripts from files and memory, converts values between Lua and
MetaCall types, and exposes Lua functions to all other supported languages.

### Requirements

- LuaJIT (>= 2.1) is required.
- The loader uses the Lua 5.1 API (`lua_setfenv`, `lua_objlen`,
  `LUA_GLOBALSINDEX`). Standard Lua 5.3+ is not supported.
- The build system finds LuaJIT through `FindLuaJIT.cmake`.

### How the Loader Works

1. `lua_loader_impl_initialize` creates a LuaJIT state and registers the
   loader types.
2. Each loaded script runs in its own environment table. This isolates the
   global namespace of each handle and prevents variable leakage between
   scripts.
3. `lua_loader_impl_discover` iterates the environment table and exposes each
   Lua function through `debug.getinfo`. Parameter counts are introspected and
   variadic functions accept up to 64 arguments.
4. `function_lua_interface_invoke` converts MetaCall values to Lua values,
   calls the function with an error handler, and converts the results back.
5. `lua_loader_impl_execution_path` appends the given path to `package.path`
   and stores it for relative file lookups.

### Value Conversion

Bidirectional conversion between Lua values and MetaCall values is implemented
in `lua_loader_convert.c`.

1. **Empty Tables**: Converted to MetaCall maps (not arrays) because array
   detection requires consecutive 1-based indexing.
2. **NaN/Infinity**: Converted to MetaCall doubles as per LuaJIT behavior.
3. **Recursion Limit**: Hardcoded at 1000 levels in `lua_loader_convert.c`.
   Tables nested beyond this limit convert to NULL and are reported through
   the log.
4. **Type Coercion**: Lua dynamic typing is preserved. Integral numbers
   become `METACALL_LONG`, non-integral numbers become `METACALL_DOUBLE`.
   In the reverse direction (MetaCall to Lua), all integer types map to
   `lua_pushinteger` and all float types map to `lua_pushnumber`.

### Number Type Detection (Lua to MetaCall)

- **Lua 5.3+**: Uses `lua_isinteger()` to detect the native integer type.
  Integers are stored as `METACALL_LONG`, floats as `METACALL_DOUBLE`.
- **Lua 5.1/5.2/LuaJIT**: These versions store all numbers as `lua_Number`
  (double). The loader checks if a number:
  1. Is finite (not NaN or infinity).
  2. Is within the safe double integer range (2^53, approximately +-9e15)
     to avoid precision loss.
  3. Fits within the platform `LONG_MIN` to `LONG_MAX` range.
  4. Has no fractional part (`n == floor(n)`).

  If all conditions are met, the value is stored as `METACALL_LONG`;
  otherwise as `METACALL_DOUBLE`.

**Note**: Values outside the safe integer range (2^53) are stored as
`METACALL_DOUBLE` even if they are whole numbers, because double cannot
exactly represent integers beyond this range on 64-bit platforms.

### Error Handling

- Load failures are logged with the failing path and its error message.
  The error is captured from the Lua VM stack before it is popped.
- Function call failures are logged with the function name and the Lua
  traceback produced by the error handler.
- `load_from_package` and `await` are not implemented and log a warning.

### Tested Environment

- OS: Linux, macOS (via CI), Windows (via CI)
- Build system: CMake
- LuaJIT version: 2.1

### Building

Use the top level CMake build flow, see `/docs/README.md` for more info.

```sh
cmake -DOPTION_BUILD_LOADERS_LUA=On ..
cmake --build . --target install
```
### Format

use `ghcr.io/jidicula/clang-format:12` to format the code.

### Testing

```sh
ctest -R "metacall-lua.*"
```

The test suites are `metacall_lua_test` (basic functionality) and
`metacall_lua_mixed_test` (edge cases). Their Lua scripts live in
`source/scripts/lua/` and are copied to the loader script path at build
time by the scripts infrastructure.
