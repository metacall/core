(module
  (memory (export "memory") 1)
  (table (export "table") 1 funcref)
  (global (export "immutable_global") i32 (i32.const 1))
  (global (export "mutable_global") (mut i32) (i32.const 1))
  (func (export "duplicate_func") (result i32)
    i32.const 1
  )
)
