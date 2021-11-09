#[no_mangle]
pub extern "C" fn add(num_1: i32, num_2: i32) -> i32 {
    num_1 + num_2
}

#[no_mangle]
pub extern "C" fn run() {
    println!("Hello World")
}
