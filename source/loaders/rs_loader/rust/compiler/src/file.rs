use crate::{compile, CompilerState, RegistrationError, Source};

use std::{ffi::c_void, path::PathBuf};

use crate::{registrator, DynlinkLibrary};

#[derive(Debug)]
pub struct FileRegistration {
    pub path_to_file: PathBuf,
    pub state: CompilerState,
    pub dynlink: Option<DynlinkLibrary>,
}
impl FileRegistration {
    pub fn new(path_to_file: PathBuf) -> Result<FileRegistration, RegistrationError> {
        let state = match compile(Source::new(Source::File {
            path: PathBuf::from(path_to_file.clone()),
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

        Ok(FileRegistration {
            path_to_file,
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
