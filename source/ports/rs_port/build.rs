use std::env;
use std::path::PathBuf;

fn generate_bindings() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"));
    let output = manifest_dir.join("src").join("bindings.rs");
    let mut builder = bindgen::Builder::default()
        .allowlist_function("metacall.*")
        .rustified_enum("metacall_.*");

    if let Ok(build_dir) = env::var("CMAKE_BINARY_DIR") {
        let build_dir = PathBuf::from(build_dir);
        let root = manifest_dir
            .ancestors()
            .nth(3)
            .expect("failed to find project root");
        let include_metacall = root.join("source/metacall/include/metacall");

        for entry in std::fs::read_dir(&include_metacall).expect("failed to read include dir") {
            let path = entry.expect("failed to read entry").path();
            if path.extension().and_then(|e| e.to_str()) == Some("h") {
                println!("cargo:rerun-if-changed={}", path.display());
                builder = builder.header(path.to_string_lossy());
            }
        }

        builder = builder
            .clang_arg(format!("-I{}", root.join("source/metacall/include").display()))
            .clang_arg(format!("-I{}", build_dir.join("source/metacall/include").display()))
            .clang_arg(format!("-I{}", build_dir.join("source/include").display()));
    } else {
        let lib = metacall_sys::find_metacall_library().unwrap();
        let include = lib.path.parent().expect("library path has no parent").join("include");
        let include_metacall = include.join("metacall");

        for entry in std::fs::read_dir(&include_metacall).expect("failed to read include dir") {
            let path = entry.expect("failed to read entry").path();
            if path.extension().and_then(|e| e.to_str()) == Some("h") {
                println!("cargo:rerun-if-changed={}", path.display());
                builder = builder.header(path.to_string_lossy());
            }
        }

        builder = builder.clang_arg(format!("-I{}", include.display()));
    }

    builder
        .generate()
        .expect("failed to generate bindings")
        .write_to_file(output)
        .expect("failed to write bindings.rs");
}

fn main() {
    // Find and link MetaCall library
    metacall_sys::build();

    // Generate bindings for current platform
    generate_bindings();
}
