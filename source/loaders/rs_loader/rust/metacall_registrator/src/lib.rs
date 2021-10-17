use std::{ffi::c_void, path::PathBuf};

pub struct CargoCdylibProject {
    path_to_project: PathBuf,
    path_to_project_librs: PathBuf,
}
impl CargoCdylibProject {
    pub fn new(
        path_buf: PathBuf,
        loader_impl: *mut c_void,
    ) -> Result<CargoCdylibProject, &'static str> {
        Err("Basic implementation")
    }

    pub fn register_the_project_in_metacall(&self) -> Result<(), &'static str> {
        Err("Basic implementation")
    }
}
