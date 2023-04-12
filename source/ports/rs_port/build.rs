use bindgen::{builder, CargoCallbacks};
use std::env;

fn generate_bindings(headers: &[&str]) {
    let mut builder = builder();

    builder = builder.clang_arg(format!(
        "-I{}",
        env::current_dir()
            .unwrap()
            .join("include")
            .to_str()
            .unwrap()
    ));

    for header in headers {
        builder = builder.header(header.to_string());
    }

    builder = builder
        .detect_include_paths(true)
        .size_t_is_usize(true)
        .rustfmt_bindings(true)
        .generate_comments(true)
        .parse_callbacks(Box::new(CargoCallbacks))
        .derive_hash(true);

    let bindings = builder.generate().unwrap();

    bindings
        .write_to_file(env::current_dir().unwrap().join("src/bindings.rs"))
        .unwrap();
}

fn main() {
    // When running from CMake
    if let Ok(_) = env::var("CMAKE_BINDGEN") {
        const HEADERS: [&str; 3] = [
            "include/metacall/metacall.h",
            "include/metacall/metacall_value.h",
            "include/metacall/metacall_error.h",
        ];

        generate_bindings(&HEADERS);
    }

    println!("cargo:rerun-if-changed=src/bindings.rs");

    // Compile time assert for validating the minimum METACALL_VERSION
    // TODO

    // When running tests from CMake
    if let Ok(val) = env::var("PROJECT_OUTPUT_DIR") {
        // Link search path to build folder
        println!("cargo:rustc-link-search={val}");

        // Set up environment variables
        if let Ok(name) = env::var("PROJECT_LIBRARY_PATH_NAME") {
            println!("cargo:rustc-env={name}={val}");
        }
        println!("cargo:rustc-env=CONFIGURATION_PATH={val}/configurations/global.json");

        // Link against correct version of metacall
        match env::var("CMAKE_BUILD_TYPE") {
            Ok(val) => {
                if val == "Debug" {
                    // try to link the debug version when running tests
                    println!("cargo:rustc-link-lib=metacalld");
                } else {
                    println!("cargo:rustc-link-lib=metacall");
                }
            }
            Err(_) => {
                println!("cargo:rustc-link-lib=metacall");
            }
        }
    } else {
        // When building from Cargo
        let profile = env::var("PROFILE").unwrap();
        match profile.as_str() {
            "debug" => {
                println!("cargo:rustc-link-lib=metacalld")
            }
            "release" => {
                println!("cargo:rustc-link-lib=metacall")
            }
            _ => {
                println!("cargo:rustc-link-lib=metacall")
            }
        }
    }
}
