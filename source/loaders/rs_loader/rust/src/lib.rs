pub use std::ffi::c_void;
pub use std::ffi::CStr;
pub use std::os::raw::c_char;
pub use std::os::raw::c_int;
pub use std::path::PathBuf;

mod lifecycle;

pub use lifecycle::rs_loader_impl_clear;
pub use lifecycle::rs_loader_impl_destroy;
pub use lifecycle::rs_loader_impl_discover;
pub use lifecycle::rs_loader_impl_execution_path;
pub use lifecycle::rs_loader_impl_initialize;
pub use lifecycle::rs_loader_impl_load_from_file;
pub use lifecycle::rs_loader_impl_load_from_memory;
pub use lifecycle::rs_loader_impl_load_from_package;

pub struct LoaderLifecycleState {
    paths: Vec<PathBuf>,
}
impl LoaderLifecycleState {
    pub fn new(paths: Vec<PathBuf>) -> LoaderLifecycleState {
        LoaderLifecycleState { paths }
    }
}
