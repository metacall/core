use std::env;

fn main() {
    // Address sanitizer
    if env::var("CMAKE_ADDRESS_SANITIZER").is_ok() {
        println!("cargo:rustc-link-lib=asan");
    }

    // Thread sanitizer
    if env::var("CMAKE_THREAD_SANITIZER").is_ok() {
        println!("cargo:rustc-link-lib=tsan");
    }

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
