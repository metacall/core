use std::env;

fn main() {
    // When running tests from CMake
    if let Ok(val) = env::var("PROJECT_OUTPUT_DIR") {
        // Link search path to build folder
        println!("cargo:rustc-link-search=native={val}");

        // Link against correct version of metacall
        match env::var("CMAKE_BUILD_TYPE") {
            Ok(val) => {
                if val == "Debug" {
                    // Try to link the debug version when running tests
                    println!("cargo:rustc-link-lib=dylib=metacalld");
                } else {
                    println!("cargo:rustc-link-lib=dylib=metacall");
                }
            }
            Err(_) => {
                println!("cargo:rustc-link-lib=dylib=metacall");
            }
        }
    } else {
        // When building from Cargo
        let profile = env::var("PROFILE").unwrap();
        match profile.as_str() {
            // "debug" => {
            //     println!("cargo:rustc-link-lib=dylib=metacalld");
            // }
            "debug" | "release" => {
                println!("cargo:rustc-link-lib=dylib=metacall")
            }
            _ => {
                println!("cargo:rustc-link-lib=dylib=metacall")
            }
        }
    }
}
