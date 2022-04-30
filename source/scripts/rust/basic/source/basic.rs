use std::collections::HashMap;
#[no_mangle]
pub extern "C" fn add(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

#[no_mangle]
pub extern "C" fn add_float(num_1: f32, num_2: f32) -> f32 {
    num_1 + num_2
}


#[no_mangle]
pub extern "C" fn run() {
    println!("Hello World")
}

#[no_mangle]
pub fn add_vec(vec: &mut Vec<i32>) -> i32 {
    vec.iter().sum()
}

#[no_mangle]
pub fn add_vec2(vec: Vec<i32>) -> i32 {
    vec.iter().sum()
}

#[no_mangle]
pub fn add_map(map: HashMap<i32, f32>) -> f32 {
    map.into_values().sum()
}
