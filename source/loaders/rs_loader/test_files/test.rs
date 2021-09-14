use std::ffi::CString;

fn add(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

fn greetings(greeting_word: &str, target: &str) -> CString {
    CString::new(greeting_word.to_owned() + target + &add(1, 3).to_string()[..]).unwrap()
}

pub extern "C" fn run() {
    println!("Greetings: {:#?}", greetings("Hello", "Para"))
}
