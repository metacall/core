use std::collections::HashMap;

fn add(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

fn add2(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

fn add_float(num_1: f32, num_2: f32) -> f32 {
    num_1 + num_2
}

fn run() {
    println!("Hello World")
}

// fn add_vec(vec: &mut Vec<i32>) -> i32 {
//     vec.iter().sum()
// }

fn add_vec2(vec: Vec<i32>) -> i32 {
    vec.iter().sum()
}

fn add_float_vec(vec: Vec<f32>) -> f32 {
    vec.iter().sum()
}

fn return_vec() -> Vec<i32> {
    vec![1, 2, 3, 4, 5]
}

fn return_map() -> HashMap<i32, f32> {
    let mut map = HashMap::new();
    map.insert(1, 1.0);
    map.insert(2, 2.0);
    map.insert(3, 3.0);
    map
}

fn add_map(map: HashMap<i32, f32>) -> f32 {
    map.into_values().sum()
}

fn string_len(s: String) -> usize {
    s.len()
}

fn new_string(idx: i32) -> String {
    format!("get number {idx}")
}
