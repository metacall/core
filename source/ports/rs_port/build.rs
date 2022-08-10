use std::env;
fn main() {
    // when running tests
    if let Ok(val) = env::var("CMAKE_BINARY_DIR") {
        println!("cargo:rustc-link-search={val}");
        // try to link the debug version when running tests
        println!("cargo:rustc-link-lib=metacalld");
        println!("cargo:rustc-env=LD_LIBRARY_PATH={val}");
        println!("cargo:rustc-env=CONFIGURATION_PATH={val}/configurations/global.json")
    } else {
        println!("cargo:rustc-link-lib=metacall");
    }

    // default install location

    // user defined location
}
