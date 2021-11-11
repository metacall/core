use crate::{compile, Source, CompilerState, RegistrationError};

use std::{
    ffi::c_void,
    path::PathBuf,
};

use dlopen;

use crate::registrator;

#[derive(Debug)]
pub struct DlopenLibrary {
    instance: dlopen::raw::Library,
}
impl DlopenLibrary {
    pub fn new(path_to_dll: &PathBuf) -> Result<DlopenLibrary, String> {
        match match dlopen::raw::Library::open(path_to_dll.clone()) {
            Ok(instance) => return Ok(DlopenLibrary { instance }),
            Err(error) => match error {
                dlopen::Error::NullCharacter(null_error) => {
                    Err(format!(
                        "Provided string could not be coverted into `{}` because it contained null character. IoError: {}",
                        "std::ffi::CString",
                        null_error
                    ))
                }
                dlopen::Error::OpeningLibraryError(io_error) => {
                    Err(format!(
                        "The dll could not be opened. IoError: {}",
                        io_error
                    ))
                }
                dlopen::Error::SymbolGettingError(io_error) => {
                    Err(format!(
                        "The symbol could not be obtained. IoError: {}",
                        io_error
                    ))
                }
                dlopen::Error::NullSymbol => {
                    Err(format!(
                        "Value of the symbol was null.",
                    ))
                }
                dlopen::Error::AddrNotMatchingDll(io_error) => {
                    Err(format!(
                        "Address could not be matched to a dynamic link library. IoError: {}",
                        io_error
                    ))
                }
            },
        } {
            Ok(dlopen_library_instance) => return Ok(dlopen_library_instance),
            Err(error) => {
                let dll_opening_error = format!(
                    "{}\nrs_loader was unable to open the dll with the following path: `{}`", 
                    error,
                    path_to_dll.to_str().unwrap()
                );

                return Err(dll_opening_error)
            }
        }
    }
}

#[derive(Debug)]
pub struct FileRegistration {
    path_to_file: PathBuf,
    state: CompilerState,
    dlopen: DlopenLibrary,
}
impl FileRegistration {
    pub fn new(path_to_file: PathBuf) -> Result<FileRegistration, RegistrationError> {
        let state = match compile(
            Source::new(Source::File {
                path: PathBuf::from(path_to_file.clone()),
            }),
        ) {
            Ok(state) => state,
            Err(error) => return Err(
                RegistrationError::CompilationError(
                    String::from(format!("{}\n{}\n{}", error.err, error.errors, error.diagnostics)),
                )
            ),
        };
        let dlopen = match DlopenLibrary::new(&state.output) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::DlopenError(error)),
        };

        Ok(FileRegistration {
            path_to_file,
            state,
            dlopen,
        })
    }

    pub fn discover(
        &self,
        loader_impl: *mut c_void,
        ctx: *mut c_void,
    ) -> Result<(), String> {
        println!("Functions: {:#?}", self.state.functions); // TODO: Remove this

        registrator::register(&self.state, loader_impl, ctx);

        Ok(())
    }
}
