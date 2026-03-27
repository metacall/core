use crate::{
    bindings::{metacall_destroy, metacall_initialize, metacall_is_initialized},
    types::MetaCallInitError,
};
use std::{env, ptr};

pub struct MetaCallDestroy(unsafe extern "C" fn());

#[cfg(target_os = "windows")]
fn ensure_python_home() {
    use std::path::Path;

    if env::var_os("PYTHONHOME").is_some() {
        return;
    }

    let Some(configured_home) = option_env!("METACALL_PYTHONHOME") else {
        return;
    };

    if configured_home.is_empty() {
        return;
    }

    let configured_home_path = Path::new(configured_home);
    let runtime_python_from_config = configured_home_path.join("runtimes").join("python");
    let runtime_python_from_parent = configured_home_path
        .parent()
        .map(|p| p.join("runtimes").join("python"));

    let selected_home = if runtime_python_from_config.is_dir() {
        runtime_python_from_config.as_os_str()
    } else if let Some(runtime_python) = runtime_python_from_parent.as_ref().filter(|p| p.is_dir())
    {
        runtime_python.as_os_str()
    } else {
        configured_home_path.as_os_str()
    };

    env::set_var("PYTHONHOME", selected_home);
}

impl Drop for MetaCallDestroy {
    fn drop(&mut self) {
        unsafe { self.0() }
    }
}

/// Initializes MetaCall. Always remember to store the output in a variable to avoid instant drop.
/// For example: ...
/// ```
/// // Initialize metacall at the top of your main function before loading your codes or
/// // calling any function.
/// let _metacall = metacall::initialize().unwrap();
///
///
/// ```
pub fn initialize() -> Result<MetaCallDestroy, MetaCallInitError> {
    #[cfg(target_os = "windows")]
    ensure_python_home();

    let code = unsafe { metacall_initialize() };

    if code != 0 {
        return Err(MetaCallInitError::new(code));
    }

    Ok(MetaCallDestroy(metacall_destroy))
}

pub fn is_initialized() -> bool {
    let initialized = unsafe { metacall_is_initialized(ptr::null_mut()) };

    if initialized == 0 {
        return true;
    }

    false
}
