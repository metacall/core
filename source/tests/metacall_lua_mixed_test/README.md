# Lua Loader Mixed Edge Case Tests

## Test Coverage

### 1. Numeric Boundary Tests (6 tests)
- **convert_integer_max_boundary**: INT64_MAX (9223372036854775807) handling
- **convert_integer_min_boundary**: INT64_MIN (-9223372036854775808) handling
- **convert_double_infinity**: Positive infinity (math.huge) conversion
- **convert_double_negative_infinity**: Negative infinity conversion
- **convert_double_nan**: NaN (0/0) conversion
- **convert_double_precision**: Floating-point precision preservation

### 2. String Edge Case Tests (5 tests)
- **convert_empty_string**: Empty string handling
- **convert_unicode_string**: UTF-8 encoded strings
- **convert_string_with_special_chars**: Escape sequences (\n, \t, \\)
- **convert_long_string**: 10KB+ string stress test

### 3. Container Edge Case Tests (8 tests)
- **convert_empty_table_as_map**: Empty Lua tables convert to MetaCall maps
- **convert_sparse_array_with_nil**: Arrays with holes (nil values)
- **convert_mixed_key_table**: Tables with numeric and string keys
- **convert_zero_indexed_table**: Zero-indexed tables (Lua is 1-based)
- **convert_negative_indexed_table**: Negative index handling
- **convert_deeply_nested_structure**: Deep nesting (within limit)
- **convert_exceeds_recursion_limit**: Beyond 10-level recursion limit
- **convert_large_array**: 1000+ element arrays

### 4. Function Call Variations (6 tests)
- **call_zero_argument_function**: Functions with no parameters
- **call_function_returning_nil**: Explicit nil returns
- **call_function_no_explicit_return**: Functions without return statement
- **call_function_multiple_returns**: Multi-value returns (1, 2, 3)
- **call_variadic_function**: Variable argument functions (...)
- **call_function_with_many_arguments**: 10+ parameter functions

### 5. Error Handling (3 tests)
- **runtime_error_propagation**: Lua error() function handling
- **runtime_error_with_nil_access**: Nil index operations
- **invalid_syntax_error**: Malformed Lua code

### 6. Resource Management (3 tests)
- **multiple_handle_isolation**: Separate environments per load
- **load_after_failed_load**: Recovery from failed loads
- **mixed_types_in_array**: Arrays with mixed type elements

### 7. Integration Tests (1 test)
- **load_and_call_external_script**: External mixed_test.lua script

## Design Decisions

1. **Empty Tables**: Converted to MetaCall maps (not arrays) because array detection requires consecutive 1-based indexing

2. **NaN/Infinity**: Converted to MetaCall doubles as per LuaJIT behavior

3. **Recursion Limit**: Hardcoded at 1000 levels in lua_loader_convert.c

4. **Handle Isolation**: Each load creates a separate environment table, preventing variable leakage

5. **Type Coercion**: Lua's dynamic typing is preserved; integral numbers become METACALL_LONG, non-integral numbers become METACALL_DOUBLE. On the reverse direction (MetaCall to Lua), all integer types map to lua_pushinteger and all float types map to lua_pushnumber.

## Implementation Details

### Number Type Detection (Lua -> MetaCall)

The Lua loader handles different Lua versions appropriately:

- **Lua 5.3+**: Uses `lua_isinteger()` to detect native integer type. Integers are stored as `METACALL_LONG`, floats as `METACALL_DOUBLE`.

- **Lua 5.1/5.2/LuaJIT**: These versions store all numbers as `lua_Number` (double). The loader checks if a number:
  1. Is finite (not NaN or infinity)
  2. Is within safe double integer range (2^53, approximately ±9e15) to avoid precision loss
  3. Fits within platform `LONG_MIN` to `LONG_MAX` range
  4. Has no fractional part (`n == floor(n)`)

  If all conditions are met, the value is stored as `METACALL_LONG`; otherwise as `METACALL_DOUBLE`.

  **Note**: Values outside the safe integer range (2^53) are stored as `METACALL_DOUBLE` even if they are whole numbers, because double cannot exactly represent integers beyond this range on 64-bit platforms.
These tests complement the existing `metacall_lua_test` by focusing on boundary conditions and error scenarios rather than basic functionality.
