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
    names: Vec<&'static str>,
}

/// Represents the match of a library when it's found
struct LibraryPath {
    path: PathBuf,
    library: String,
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
        // Defaults to path: C:\Users\Default\AppData\Local
        let local_app_data = env::var("LOCALAPPDATA")
            .unwrap_or_else(|_| String::from("C:\\Users\\Default\\AppData\\Local"));

        Ok(InstallPath {
            paths: vec![PathBuf::from(local_app_data)
                .join("MetaCall")
                .join("metacall")],
            names: vec!["metacall.lib"],
        })
    } else if cfg!(target_os = "macos") {
        Ok(InstallPath {
            paths: vec![
                PathBuf::from("/opt/homebrew/lib/"),
                PathBuf::from("/usr/local/lib/"),
            ],
            names: vec!["libmetacall.dylib"],
        })
    } else if cfg!(target_os = "linux") {
        Ok(InstallPath {
            paths: vec![PathBuf::from("/usr/local/lib/"), PathBuf::from("/gnu/lib/")],
            names: vec!["libmetacall.so"],
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
            names: vec![
                "libmetacall.so",
                "libmetacalld.so",
                "libmetacall.dylib",
                "libmetacalld.dylib",
                "metacall.lib",
                "metacalld.lib",
            ],
        });
    }

    // Fall back to platform-specific paths
    platform_install_paths()
}

/// Get the parent path and library name
fn get_parent_and_library(path: &Path) -> Option<(PathBuf, String)> {
    let parent = path.parent()?.to_path_buf();

    // Get the file stem (filename without extension)
    let stem = path.file_stem()?.to_str()?;

    // Remove "lib" prefix if present
    let cleaned_stem = stem.strip_prefix("lib").unwrap_or(stem).to_string();

    Some((parent, cleaned_stem))
}

/// Find the MetaCall library
/// This orchestrates the search process
fn find_metacall_library() -> Result<LibraryPath, Box<dyn std::error::Error>> {
    let search_config = get_search_config()?;

    // Search in each configured path
    for search_path in &search_config.paths {
        for name in &search_config.names {
            // Search with no limit in depth
            match find_files_recursively(search_path, name, None) {
                Ok(files) if !files.is_empty() => {
                    let found_lib = fs::canonicalize(&files[0])?;

                    match get_parent_and_library(&found_lib) {
                        Some((parent, name)) => {
                            return Ok(LibraryPath {
                                path: parent,
                                library: name,
                            })
                        }
                        None => continue,
                    };
                }
                Ok(_) => {
                    // No files found in this path, continue searching
                    continue;
                }
                Err(e) => {
                    eprintln!("Error searching in {}: {}", search_path.display(), e);
                    continue;
                }
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

fn define_library_search_path(env_var: &str, separator: &str, path: &PathBuf) -> String {
    // Get the current value of the env var, if any
    let existing = env::var(env_var).unwrap_or_default();
    let path_str: String = String::from(path.to_str().unwrap());

    // Append to it
    let combined = if existing.is_empty() {
        path_str
    } else {
        format!("{}{}{}", existing, separator, path_str)
    };

    format!("{}={}", env_var, combined)
}

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
        // When building from Cargo, try to find MetaCall
        match find_metacall_library() {
            Ok(lib_path) => {
                // Define linker flags
                println!("cargo:rustc-link-search=native={}", lib_path.path.display());
                println!("cargo:rustc-link-lib=dylib={}", lib_path.library);

                // Set the runtime environment variable for finding the library during tests
                #[cfg(target_os = "linux")]
                const ENV_VAR: &str = "LD_LIBRARY_PATH";

                #[cfg(target_os = "macos")]
                const ENV_VAR: &str = "DYLD_LIBRARY_PATH";

                #[cfg(target_os = "windows")]
                const ENV_VAR: &str = "PATH";

                #[cfg(target_os = "aix")]
                const ENV_VAR: &str = "LIBPATH";

                #[cfg(any(target_os = "linux", target_os = "macos", target_os = "aix"))]
                const SEPARATOR: &str = ":";

                #[cfg(target_os = "windows")]
                const SEPARATOR: &str = ";";

                println!(
                    "cargo:rustc-env={}",
                    define_library_search_path(ENV_VAR, SEPARATOR, &lib_path.path)
                );
            }
            Err(e) => {
                // Print the error
                eprintln!(
                    "Failed to find MetaCall library with: {e} \
                    Still trying to link in case the library is in system paths"
                );

                // Still try to link in case the library is in system paths
                println!("cargo:rustc-link-lib=dylib=metacall")
            }
        }
    }
}
