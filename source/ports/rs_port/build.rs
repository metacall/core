use std::{
    env, fs,
    path::{Path, PathBuf},
    vec,
};

// Search for MetaCall libraries in platform-specific locations
// Handle custom installation paths via environment variables
// Find configuration files recursively
// Provide helpful error messages when things aren't found

/// Represents the install paths for a platform
struct InstallPath {
    paths: Vec<PathBuf>,
    name: String,
}

/// Find files recursively in a directory matching a pattern
fn find_files_recursively<P: AsRef<Path>>(
    root_dir: P,
    filename: &str,
    max_depth: Option<usize>,
) -> Result<Vec<PathBuf>, Box<dyn std::error::Error>> {
    let mut matches = Vec::new();
    let mut stack = vec![(root_dir.as_ref().to_path_buf(), 0)];

    while let Some((current_dir, depth)) = stack.pop() {
        if let Some(max) = max_depth {
            if depth > max {
                continue;
            }
        }

        if let Ok(entries) = fs::read_dir(&current_dir) {
            for entry in entries.flatten() {
                let path = entry.path();

                if path.is_file() {
                    // Simple filename comparison instead of regex
                    if let Some(file_name) = path.file_name().and_then(|n| n.to_str()) {
                        if file_name == filename {
                            matches.push(path);
                        }
                    }
                } else if path.is_dir() {
                    stack.push((path, depth + 1));
                }
            }
        }
    }

    Ok(matches)
}

fn platform_install_paths() -> Result<InstallPath, Box<dyn std::error::Error>> {
    if cfg!(target_os = "windows") {
        // defaults to path:
        // C:\Users\Default\AppData\Local
        let local_app_data = env::var("LOCALAPPDATA")
            .unwrap_or_else(|_| String::from("C:\\Users\\Default\\AppData\\Local"));

        println!("windows");
        Ok(InstallPath {
            paths: vec![PathBuf::from(local_app_data)
                .join("Metacall")
                .join("metacall")],
            name: "metacall.dll".to_string(),
        })
    } else if cfg!(target_os = "macos") {
        println!("macos");
        Ok(InstallPath {
            paths: vec![
                PathBuf::from("/opt/homebrew/lib/"),
                PathBuf::from("/usr/local/lib/"),
            ],
            name: "metacall.dylib".to_string(),
        })
    } else if cfg!(target_os = "linux") {
        println!("linux");
        Ok(InstallPath {
            paths: vec![PathBuf::from("/usr/local/lib/"), PathBuf::from("/gnu/lib/")],
            name: "libmetacall.so".to_string(),
        })
    } else {
        Err(format!("Platform {} not supported", env::consts::OS).into())
    }
}

/// Get search paths, checking for custom installation path first
fn get_search_config() -> Result<InstallPath, Box<dyn std::error::Error>> {
    // First, check if user specified a custom path
    if let Ok(custom_path) = env::var("METACALL_INSTALL_PATH") {
        // For custom paths, we need to search for any metacall library variant
        return Ok(InstallPath {
            paths: vec![PathBuf::from(custom_path)],
            name: r"^(lib)?metacall(d)?\.(so|dylib|dll)$".to_string(),
        });
    }

    // Fall back to platform-specific paths
    platform_install_paths()
}

/// Find the MetaCall library
/// This orchestrates the search process
fn find_metacall_library() -> Result<PathBuf, Box<dyn std::error::Error>> {
    let search_config = get_search_config()?;

    // Search in each configured path
    for search_path in &search_config.paths {
        println!(
            "cargo:warning=Searching for MetaCall in: {}",
            search_path.display()
        );

        // Only search at depth 0 (current directory)
        match find_files_recursively(search_path, &search_config.name, Some(0)) {
            Ok(files) if !files.is_empty() => {
                let found_lib = &files[0];
                println!(
                    "cargo:warning=Found MetaCall library: {}",
                    found_lib.display()
                );
                return Ok(found_lib.clone());
            }
            Ok(_) => {
                // No files found in this path, continue searching
                continue;
            }
            Err(e) => {
                println!(
                    "cargo:warning=Error searching in {}: {}",
                    search_path.display(),
                    e
                );
                continue;
            }
        }
    }

    // If we get here, library wasn't found
    let search_paths: Vec<String> = search_config
        .paths
        .iter()
        .map(|p| p.display().to_string())
        .collect();

    Err(format!(
        "MetaCall library not found. Searched in: {}. \
        If you have it installed elsewhere, set METACALL_INSTALL_PATH environment variable.",
        search_paths.join(", ")
    )
    .into())
}

fn main() {
    println!("------------ BEGIN ------------");

    // When running tests from CMake
    if let Ok(val) = env::var("PROJECT_OUTPUT_DIR") {
        println!("cargo:warning=Using CMake build path: {}", val);
        println!("cargo:rustc-link-search=native={val}");

        match env::var("CMAKE_BUILD_TYPE") {
            Ok(val) if val == "Debug" => {
                println!("cargo:rustc-link-lib=dylib=metacalld");
            }
            _ => {
                println!("cargo:rustc-link-lib=dylib=metacall");
            }
        }
        return;
    }

    println!("cargo:warning=Using pure Cargo build, searching for MetaCall...");

    // When building from Cargo - try to find MetaCall
    match find_metacall_library() {
        Ok(lib_path) => {
            // Extract the directory containing the library
            if let Some(lib_dir) = lib_path.parent() {
                println!("cargo:rustc-link-search=native={}", lib_dir.display());
            }

            // Link against the library
            let profile = env::var("PROFILE").unwrap_or_else(|_| "release".to_string());
            match profile.as_str() {
                "debug" | "release" => {
                    println!("cargo:rustc-link-lib=dylib=metacall");
                }
                _ => {
                    println!("cargo:rustc-link-lib=dylib=metacall");
                }
            }
        }
        Err(e) => {
            println!("cargo:warning={e}");
            // Still try to link in case the library is in system paths
            println!("cargo:rustc-link-lib=dylib=metacall");
        }
    }
}
