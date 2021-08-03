(module
  (import "exports1" "memory" (memory 1))
  (import "exports1" "table" (table 1 funcref))
  (import "exports1" "immutable_global" (global i32))
  (import "exports1" "mutable_global" (global (mut i32)))

  (import "exports1" "duplicate_func" (func $duplicate_func_i32 (result i32)))
  (import "exports2" "duplicate_func" (func $duplicate_func_i64 (result i64)))

  (func (export "duplicate_func_i32") (result i32) (call $duplicate_func_i32))
  (func (export "duplicate_func_i64") (result i64) (call $duplicate_func_i64))
)
