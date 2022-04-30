use crate::{compile, CompilerState, RegistrationError, Source};

use std::{ffi::c_void, fs};

use crate::{registrator, DlopenLibrary};

#[derive(Debug)]
pub struct MemoryRegistration {
    pub name: String,
    pub state: CompilerState,
    pub dlopen: DlopenLibrary,
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
        let dlopen = match DlopenLibrary::new(&state.output) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::DlopenError(error)),
        };
        // delete compiled library
        fs::remove_file(&state.output).expect("unable to delete compiled library");

        Ok(MemoryRegistration {
            name,
            state,
            dlopen,
        })
    }

    pub fn discover(&self, loader_impl: *mut c_void, ctx: *mut c_void) -> Result<(), String> {
        registrator::register(&self.state, &self.dlopen, loader_impl, ctx);

        Ok(())
    }
}
