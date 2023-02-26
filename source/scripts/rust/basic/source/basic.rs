use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::Write;

pub fn add(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

pub fn add2(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

pub fn add_float(num_1: f32, num_2: f32) -> f32 {
    num_1 + num_2
}

pub fn run() {
    let dir = env::temp_dir();
    let mut f = File::create(dir.join("hello.txt")).unwrap();
    let _ = f.write(b"Hello metacall");
}

// pub fn add_vec(vec: &mut Vec<i32>) -> i32 {
//     vec.iter().sum()
// }

pub fn add_vec2(vec: Vec<i32>) -> i32 {
    vec.iter().sum()
}

pub fn add_float_vec(vec: Vec<f32>) -> f32 {
    vec.iter().sum()
}

pub fn return_vec() -> Vec<i32> {
    vec![1, 2, 3, 4, 5]
}

pub fn return_map() -> HashMap<i32, f32> {
    let mut map = HashMap::new();
    map.insert(1, 1.0);
    map.insert(2, 2.0);
    map.insert(3, 3.0);
    map
}

pub fn add_map(map: HashMap<i32, f32>) -> f32 {
    map.into_values().sum()
}

pub fn string_len(s: String) -> usize {
    s.len()
}

pub fn new_string(idx: i32) -> String {
    format!("get number {idx}")
}

pub fn str_slice(s: &str) -> &str {
    if s.len() < 4 {
        return s;
    } else {
        &s[0..3]
    }
}

// TODO
/*
pub fn age_group(age: i32) -> Result<String, String> {
    let valid_age = match age {
        _ if age < 0 => return Err("not born yet".to_string()),
        _ if age > 150 => return Err("seriously?!!".to_string()),
        validated => validated,
    };

    let result = match valid_age {
        _ if age < 10 => "child".to_string(),
        _ if age >= 18 => "adult".to_string(),
        a => format!("teenager of {} years old", a),
    };

    Ok(result)
}
*/
