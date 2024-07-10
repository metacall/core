use metacall::{loaders, metacall_no_arg, switch};
use std::{
    env,
    fs::{self, File},
    io::Write,
    path::PathBuf,
};

// Two different names to avoid conflicts when testing both load_from_memory and load_from_files
// in a single test.
const SCRIPT1: &str = "function greet1() { return 'hi there!' } \nmodule.exports = { greet1 }";
const SCRIPT2: &str = "function greet2() { return 'hi there!' } \nmodule.exports = { greet2 };";
const SCRIPT3: &str = "console.log('Hello world')";
fn call_greet(test: &str, num: u32) {
    let out = metacall_no_arg::<String>(format!("greet{}", num)).unwrap();
    if out.as_str() == "hi there!" {
        return ();
    } else {
        panic!(
            "Invalid output of the function! Expected `hi there!` but received `{}`.",
            test
        );
    }
}

fn load_from_memory_test() {
    loaders::from_memory("node", SCRIPT1).unwrap();
    call_greet("load_from_memory", 1);

    loaders::from_memory("node", SCRIPT3).unwrap();
}

fn load_from_file_test() {
    // Finding a temporary address to store the temporary js file
    let temp_js_pathbuf =
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap()).join("target/tmp/greet.js");
    let temp_js_path = temp_js_pathbuf.to_str().unwrap();

    // Writing to that file
    let mut temp_js = File::create(&temp_js_pathbuf).unwrap();
    temp_js.write_all(SCRIPT2.as_bytes()).unwrap();
    temp_js.flush().unwrap();

    loaders::from_single_file("node", temp_js_path).unwrap();

    call_greet("load_from_file", 2);

    // Removing that file
    fs::remove_file(temp_js_pathbuf).unwrap();
}

#[test]
fn loaders() {
    let _d = switch::initialize().unwrap();
    // Testing load_from_memory
    load_from_memory_test();

    // Testing load_from_files
    load_from_file_test();
}
