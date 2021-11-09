use crate::{Source, compile, Parser, RegistrationError};

use std::{
    ffi::c_void,
    path::PathBuf,
    fs,
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
    parser: Parser,
    path_to_dll: PathBuf,
    path_to_file: PathBuf,
    dlopen: DlopenLibrary,
}
impl FileRegistration {
    fn compile_to_dll(path_to_file: &PathBuf) -> Result<PathBuf, String> {
        compile(
            Source::new(
                fs::read_to_string(path_to_file.clone()).unwrap(),
                Some(PathBuf::from(path_to_file.clone().parent().unwrap())),
                PathBuf::from(path_to_file.file_name().unwrap()),
            ),
        )
    }

    pub fn new(path_to_file: PathBuf) -> Result<FileRegistration, RegistrationError> {
        let path_to_file = path_to_file.clone();
        let path_to_dll = match FileRegistration::compile_to_dll(&path_to_file) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::ValidationError(error)),
        };
        let parser = match Parser::new(&path_to_file) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::SynError(error)),
        };
        let dlopen = match DlopenLibrary::new(&path_to_dll) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::DlopenError(error)),
        };

        Ok(FileRegistration {
            dlopen,
            parser,
            path_to_dll,
            path_to_file,
        })
    }

    pub fn register_in_metacall(
        &self,
        loader_impl: *mut c_void,
        ctx: *mut c_void,
    ) -> Result<(), String> {
        println!("Parser ast: {:#?}", self.parser.ast);

        registrator::register(&self.parser.ast, loader_impl, ctx);

        Ok(())
    }
}
