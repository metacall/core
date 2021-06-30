(module
  (func (export "none_ret_none"))
  (func (export "i32_ret_none") (param i32))
  (func (export "i32_f32_i64_f64_ret_none") (param i32) (param f32) (param i64) (param f64))
  (func (export "none_ret_i32") (result i32)
        i32.const 1)
  (func (export "none_ret_i32_f32_i64_f64") (result i32 f32 i64 f64)
        i32.const 1
        f32.const 2
        i64.const 3
        f64.const 4)
  (func (export "i32_f32_i64_f64_ret_i32_f32_i64_f64") (param i32) (param f32) (param i64) (param f64) (result i32 f32 i64 f64)
        i32.const 1
        f32.const 2
        i64.const 3
        f64.const 4)
)
