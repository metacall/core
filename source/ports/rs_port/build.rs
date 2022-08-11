use std::env;
fn main() {
    // when running tests
    if let Ok(val) = env::var("CMAKE_BINARY_DIR") {
        println!("cargo:rustc-link-search={val}");

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

        println!("cargo:rustc-env=LD_LIBRARY_PATH={val}");
        println!("cargo:rustc-env=CONFIGURATION_PATH={val}/configurations/global.json")
    } else {
        println!("cargo:rustc-link-lib=metacall");
    }

    // default install location

    // user defined location
}
