use bindgen::builder;
use git2::Repository;
use std::{
    env, fs, io,
    path::{Path, PathBuf},
};

const METACALL_CORE_REPO: &str = "https://github.com/metacall/core.git#v0.7.3";

pub fn copy_recursively(source: impl AsRef<Path>, destination: impl AsRef<Path>) -> io::Result<()> {
    fs::create_dir_all(&destination)?;
    for entry in fs::read_dir(source)? {
        let entry = entry?;
        let filetype = entry.file_type()?;

        if filetype.is_dir() {
            copy_recursively(entry.path(), destination.as_ref().join(entry.file_name()))?;
        } else {
            fs::copy(entry.path(), destination.as_ref().join(entry.file_name()))?;
        }
    }
    Ok(())
}
fn get_bindings_dir() -> PathBuf {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let bindings_dir = out_dir.join("bindings");

    fs::create_dir_all(&bindings_dir).unwrap();

    bindings_dir.canonicalize().unwrap()
}
fn download_the_headers<T: ToString>(bindings_dir: &PathBuf, headers: &[T]) {
    let mut need_for_clone = false;
    let bindings_str = bindings_dir.to_str().unwrap();

    for header in headers {
        let path = PathBuf::from(format!("{}/{}", bindings_str, header.to_string()));
        if !path.exists() && !need_for_clone {
            need_for_clone = true;
        }
    }

    if need_for_clone {
        let temp_dir = tempfile::tempdir().unwrap();
        Repository::clone(METACALL_CORE_REPO, &temp_dir).unwrap();

        let tmp_dir = temp_dir.path().to_str().unwrap();
        let source = format!("{}/source/metacall", tmp_dir);
        let destination = bindings_dir.join("metacall");
        copy_recursively(source, destination).unwrap();
    }
}
fn generate_bindings<T: ToString>(bindings_dir: &PathBuf, headers: &[T]) {
    let bindings_dir_str = bindings_dir.to_str().unwrap();
    let mut builder = builder();

    for header in headers {
        builder = builder.header(format!("{}/{}", bindings_dir_str, header.to_string()));
    }

    builder = builder
        .detect_include_paths(true)
        .size_t_is_usize(true)
        .rustfmt_bindings(true)
        .generate_comments(true)
        .derive_hash(true);
    let bindings = builder.generate().unwrap();

    bindings
        .write_to_file(bindings_dir.join("bindings.rs"))
        .unwrap();
}

fn main() {
    const HEADERS: [&str; 3] = [
        "metacall/include/metacall/metacall.h",
        "metacall/include/metacall/metacall_value.h",
        "metacall/include/metacall/metacall_error.h",
    ];
    let bindings_dir = get_bindings_dir();

    download_the_headers(&bindings_dir, &HEADERS);
    generate_bindings(&bindings_dir, &HEADERS);

    for header in HEADERS {
        println!(
            "{}",
            format!(
                "cargo:rerun-if-changed={}/{}",
                bindings_dir.to_str().unwrap(),
                header
            )
        );
    }

    // when running tests
    if let Ok(val) = env::var("PROJECT_OUTPUT_DIR") {
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

        if let Ok(name) = env::var("PROJECT_LIBRARY_PATH_NAME") {
            println!("cargo:rustc-env={name}={val}");
        }
        println!("cargo:rustc-env=CONFIGURATION_PATH={val}/configurations/global.json")
    } else {
        let profile = env::var("PROFILE").unwrap();
        match profile.as_str() {
            "debug" => {
                println!("cargo:rustc-link-lib=metacalld")
            },
            "release" => {
                println!("cargo:rustc-link-lib=metacall")
            },
            _ => {
                println!("cargo:rustc-link-lib=metacall")
            },
        }
    }
}
