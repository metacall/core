use std::collections::HashMap;

fn identity_bool(x: bool) -> bool { x }
fn identity_char(x: i8) -> i8 { x }
fn identity_short(x: i16) -> i16 { x }
fn identity_int(x: i32) -> i32 { x }
fn identity_long(x: i64) -> i64 { x }
fn identity_float(x: f32) -> f32 { x }
fn identity_double(x: f64) -> f64 { x }
fn identity_string(x: String) -> String { x }
fn identity_array(x: Vec<i32>) -> Vec<i32> { x }
fn identity_map(x: HashMap<String, String>) -> HashMap<String, String> { x }