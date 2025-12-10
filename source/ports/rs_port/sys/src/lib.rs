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
    /// Path to the library for linking (where .lib/.so/.dylib is)
    path: PathBuf,
    /// Library name for linking
    library: String,
    /// Path for runtime search (where .dll/.so/.dylib is for PATH/LD_LIBRARY_PATH)
    search: PathBuf,
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
                    println!(
                        "cargo:warning=MetaCall Searching Library: {}",
                        path.display(),
                    );

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
            names: vec!["metacall.lib", "metacalld.lib"],
        })
    } else if cfg!(target_os = "macos") {
        Ok(InstallPath {
            paths: vec![
                PathBuf::from("/opt/homebrew/lib/"),
                PathBuf::from("/usr/local/lib/"),
            ],
            names: vec!["libmetacall.dylib, libmetacalld.dylib"],
        })
    } else if cfg!(target_os = "linux") {
        Ok(InstallPath {
            paths: vec![PathBuf::from("/usr/local/lib/"), PathBuf::from("/gnu/lib/")],
            names: vec!["libmetacall.so", "libmetacalld.so"],
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

/// Strip the Windows extended-length path prefix (\\?\) if present
/// fs::canonicalize() on Windows returns paths with this prefix which can cause issues
#[cfg(target_os = "windows")]
fn strip_extended_length_prefix(path: PathBuf) -> PathBuf {
    let path_str = path.to_string_lossy();
    if let Some(stripped) = path_str.strip_prefix(r"\\?\") {
        PathBuf::from(stripped)
    } else {
        path
    }
}

/// Find the runtime DLL on Windows
/// This searches for metacall.dll or metacalld.dll recursively
#[cfg(target_os = "windows")]
fn find_metacall_dll(
    search_paths: &[PathBuf],
    library_name: &str,
) -> Result<PathBuf, Box<dyn std::error::Error>> {
    // Determine the DLL name based on the library name (metacall or metacalld)
    let dll_name = format!("{}.dll", library_name);

    for search_path in search_paths {
        match find_files_recursively(search_path, &dll_name, None) {
            Ok(files) if !files.is_empty() => {
                let found_dll = fs::canonicalize(&files[0])?;
                if let Some(parent) = found_dll.parent() {
                    return Ok(strip_extended_length_prefix(parent.to_path_buf()));
                }
            }
            _ => continue,
        }
    }

    Err(format!(
        "MetaCall DLL ({}) not found. Searched in: {}",
        dll_name,
        search_paths
            .iter()
            .map(|p| p.display().to_string())
            .collect::<Vec<_>>()
            .join(", ")
    )
    .into())
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
                        Some((parent, library_name)) => {
                            // On Windows, strip the extended-length path prefix and find DLL separately
                            #[cfg(target_os = "windows")]
                            let (lib_path, search_path) = {
                                let cleaned_parent = strip_extended_length_prefix(parent);
                                let dll_search = match find_metacall_dll(
                                    &search_config.paths,
                                    &library_name,
                                ) {
                                    Ok(dll_path) => dll_path,
                                    Err(e) => {
                                        println!(
                                            "cargo:warning=Could not find DLL, using library path: {}",
                                            e
                                        );
                                        cleaned_parent.clone()
                                    }
                                };
                                (cleaned_parent, dll_search)
                            };

                            // On non-Windows platforms, the shared library is used for both
                            // linking and runtime, so search path is the same as lib path
                            #[cfg(not(target_os = "windows"))]
                            let (lib_path, search_path) = (parent.clone(), parent);

                            return Ok(LibraryPath {
                                path: lib_path,
                                library: library_name,
                                search: search_path,
                            });
                        }
                        None => continue,
                    };
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

fn define_library_search_path(env_var: &str, separator: &str, path: &Path) -> String {
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

/// Set RPATH for runtime library discovery
/// This binaries work outside cargo
fn set_rpath(lib_path: &Path) {
    let path_str = lib_path.to_str().unwrap();

    #[cfg(target_os = "linux")]
    {
        // On Linux, use RPATH with $ORIGIN for relocatable binaries
        println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path_str);
        // Also set a backup rpath relative to the executable location
        println!("cargo:rustc-link-arg=-Wl,-rpath,$ORIGIN");
        println!("cargo:rustc-link-arg=-Wl,-rpath,$ORIGIN/../lib");
    }

    #[cfg(target_os = "macos")]
    {
        // On macOS, use @rpath and @loader_path
        println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path_str);
        // Also set loader-relative paths for relocatable binaries
        println!("cargo:rustc-link-arg=-Wl,-rpath,@loader_path");
        println!("cargo:rustc-link-arg=-Wl,-rpath,@loader_path/../lib");
    }

    #[cfg(target_os = "aix")]
    {
        // Add default system library paths to avoid breaking standard lookup
        println!(
            "cargo:rustc-link-arg=-Wl,-blibpath:{}:/usr/lib:/lib",
            path_str
        );
    }

    #[cfg(target_os = "windows")]
    {
        // Windows doesn't use RPATH, but we can inform the user
        println!(
            "cargo:warning=On Windows, make sure {} is in your PATH or next to your executable",
            path_str
        );
    }
}

pub fn build() {
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

                // Set RPATH so the binary can find libraries at runtime
                set_rpath(&lib_path.path);

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
                    define_library_search_path(ENV_VAR, SEPARATOR, &lib_path.search)
                );

                println!(
                    "cargo:warning=Library {} found in: {} with runtime search path: {}",
                    lib_path.library,
                    lib_path.path.display(),
                    lib_path.search.display()
                );
            }
            Err(e) => {
                // Print the error
                println!(
                    "cargo:warning=Failed to find MetaCall library with: {e} \
                    Still trying to link in case the library is in system paths"
                );

                // Still try to link in case the library is in system paths
                println!("cargo:rustc-link-lib=dylib=metacall")
            }
        }
    }
}
