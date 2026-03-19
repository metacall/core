use crate::{compile, CompilerState, RegistrationError, Source};

use std::{ffi::c_void};

use crate::{registrator, DynlinkLibrary};

#[derive(Debug)]
pub struct MemoryRegistration {
    pub name: String,
    pub state: CompilerState,
    pub dynlink: Option<DynlinkLibrary>,
}
impl MemoryRegistration {
    pub fn new(name: String, code: String) -> Result<MemoryRegistration, RegistrationError> {
        let state = match compile(Source::new(Source::Memory {
            name: name.clone(),
            code,
        })) {
            Ok(state) => state,
            Err(error) => {
                return Err(RegistrationError::CompilationError(String::from(format!(
                    "{}\n{}\n{}",
                    error.err, error.errors, error.diagnostics
                ))))
            }
        };
        let dynlink = match DynlinkLibrary::new(&state.output) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::DynlinkError(error)),
        };
        Ok(MemoryRegistration {
            name,
            state,
            dynlink: Some(dynlink),
        })
    }

    pub fn discover(&self, loader_impl: *mut c_void, ctx: *mut c_void) -> Result<(), String> {
        match &self.dynlink {
            Some(dl) => {
                registrator::register(&self.state, &dl, loader_impl, ctx);
                Ok(())
            }
            None => Err(String::from("The Dynlink library is None")),
        }
    }
}
impl Drop for MemoryRegistration {
    fn drop(&mut self) {
        drop(std::mem::replace(&mut self.dynlink, None));

        let mut path = std::mem::take(&mut self.state.output);
        if path.pop() {
            let _ = std::fs::remove_dir_all(path);
        }
    }
}