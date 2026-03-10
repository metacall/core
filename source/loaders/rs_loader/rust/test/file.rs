// This is only used for testing
use std::collections::HashMap;
use std::os::raw::c_void;

pub fn multiply(a: i32, b: i32) -> i32 {
    a * b
}

pub fn map_string_string(m: HashMap<String, String>) -> HashMap<String, String> {
    m.into_iter().map(|(k, v)| (k + "1", v + "1")).collect()
}

pub fn map_string_int(m: HashMap<String, i32>) -> HashMap<String, i32> {
    m.into_iter().map(|(k, v)| (k + "1", v + 1)).collect()
}

pub fn map_int_string(m: HashMap<i32, String>) -> HashMap<i32, String> {
    m.into_iter().map(|(k, v)| (k + 1, v + "1")).collect()
}

pub fn return_ptr(p: *mut c_void) -> *mut c_void {
    p
}