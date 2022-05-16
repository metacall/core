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
pub fn return_vec() -> Vec<i32> {
    vec![1, 2, 3, 4, 5]
}

#[no_mangle]
pub fn return_map() -> HashMap<i32, f32> {
    let mut map = HashMap::new();
    map.insert(1, 1.0);
    map.insert(2, 2.0);
    map.insert(3, 3.0);
    map
}

#[no_mangle]
pub fn add_map(map: HashMap<i32, f32>) -> f32 {
    map.into_values().sum()
}

#[no_mangle]
pub fn string_len(s: String) -> usize {
    s.len()
}

#[no_mangle]
pub fn new_string(idx: i32) -> String {
    format!("get number {idx}")
}

#[repr(C)]
struct Book {
    name: String,
    price: u32,
}

impl Book {
    fn new(name: String, price: u32) -> Self {
        Self { name, price }
    }
    fn get_price(&self) -> u32 {
        self.price
    }
}

impl Drop for Book {
    fn drop(&mut self) {}
}
trait BookTrait {
    fn buy_book(&self, p: i32);
}
impl BookTrait for Book {
    fn buy_book(&self, p: i32) {}
}
