use crate::{Parser, RegistrationError};

use std::{
    ffi::c_void,
    io::{BufRead, BufReader},
    path::PathBuf,
    process::{Command, Stdio},
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
        let path_to_file_directory = PathBuf::from(path_to_file.clone().parent().unwrap());
        let output_directory = format!("--out-dir={}", path_to_file_directory.to_str().unwrap());

        let path_to_file_stem =
            String::from(path_to_file.clone().file_stem().unwrap().to_str().unwrap());

        let mut path_to_dll = path_to_file_directory.clone();
        path_to_dll.push(format!("lib{}", path_to_file_stem));
        path_to_dll.set_extension("so");

        if !path_to_dll.exists() || !path_to_dll.is_file() {
            return Err(String::from(
                "Faild to compile the source file to a shared library object",
            ));
        }

        let stdout = Command::new("rustc")
            .arg(output_directory)
            .arg("--crate-type=cdylib")
            .arg(path_to_file.to_str().unwrap())
            .stdout(Stdio::piped())
            .spawn()
            .unwrap()
            .stdout
            .ok_or_else(|| "Could not capture standard output.")
            .unwrap();

        let reader = BufReader::new(stdout);

        reader
            .lines()
            .filter_map(|line| line.ok())
            .for_each(|line| println!("Line is: {:#?}", line));

        Ok(path_to_dll)
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
