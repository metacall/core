use compiler;
use std::fs;

fn main() {
    let filename = String::from("test_files/test.rs");
    let code = fs::read_to_string(&filename).unwrap();
    let source = compiler::Source::new(code, filename);

    compiler::compile(source);
}
