use compiler::api;
use compiler::file::FileRegistration;
use compiler::memory::MemoryRegistration;
use compiler::package::PackageRegistration;

use std::ffi::CStr;
use std::fmt::Display;
use std::os::raw::{c_char, c_int, c_void};
use std::path::PathBuf;

#[derive(Debug)]
pub enum LoadingMethod {
    File(FileRegistration),
    Package(PackageRegistration),
    Memory(MemoryRegistration),
}

impl LoadingMethod {
    pub fn consume_dlib(self) -> Result<compiler::DlopenLibrary, String> {
        match self {
            Self::File(FileRegistration { mut dlopen, .. }) => match dlopen {
                Some(_) => {
                    let dl = std::mem::replace(&mut dlopen, None);
                    Ok(dl.expect("Unexpected: dlopen is None"))
                }
                None => Err(String::from("consume_dlib was called more than once")),
            },
            Self::Package(PackageRegistration { mut dlopen, .. }) => match dlopen {
                Some(_) => {
                    let dl = std::mem::replace(&mut dlopen, None);
                    Ok(dl.expect("Unexpected: dlopen is None"))
                }
                None => Err(String::from("consume_dlib was called more than once")),
            },
            Self::Memory(MemoryRegistration { mut dlopen, .. }) => match dlopen {
                Some(_) => {
                    let dl = std::mem::replace(&mut dlopen, None);
                    Ok(dl.expect("Unexpected: dlopen is None"))
                }
                None => Err(String::from("consume_dlib was called more than once")),
            },
        }
    }
}

// Trait aliasing
pub trait OnPathBufClosure:
    Fn(PathBuf, fn(error: String) -> *mut c_void) -> Result<LoadingMethod, *mut c_void>
{
}
impl<T: Fn(PathBuf, fn(error: String) -> *mut c_void) -> Result<LoadingMethod, *mut c_void>>
    OnPathBufClosure for T
{
}

pub type LoadOnErrorPointer = fn(error: String) -> *mut c_void;

pub fn load_on_error<T: Display>(error: T) -> *mut c_void {
    eprintln!("{}", error);

    0 as c_int as *mut c_void
}

pub fn load<T>(
    loader_impl: *mut c_void,
    loadable_path: *mut *const c_char,
    size: usize,
    path_is_vector: bool,
    on_path_buf: T,
) -> *mut c_void
where
    T: OnPathBufClosure,
{
    let loader_lifecycle_state = unsafe {
        api::get_loader_lifecycle_state(loader_impl)
            .as_mut()
            .expect("Unable to get lifecycle state.")
    };
    let mut execution_paths_iterator = loader_lifecycle_state.execution_paths.iter();

    let mut path: *const c_char;

    let mut handle_shared_object = Box::new(Vec::new());

    for i in 0..size {
        if path_is_vector {
            path = loadable_path.wrapping_offset(i as isize) as *const c_char
        } else {
            path = loadable_path as *const i8;
        }

        let path_slice = unsafe { CStr::from_ptr(path) }
            .to_str()
            .expect("Unable to cast CStr to str");
        let mut path_buf = PathBuf::from(path_slice);

        if !path_buf.is_absolute() {
            loop {
                let execution_path_current_iteration = execution_paths_iterator.next();
                let original_path_buf = path_buf.clone();

                match execution_path_current_iteration {
                    Some(original_execution_path) => {
                        let mut execution_path = original_execution_path.clone();
                        let mut execution_path_as_str =
                            execution_path.to_str().expect("Unable to cast CStr to str");

                        if !execution_path_as_str.ends_with("/") {
                            execution_path =
                                PathBuf::from(format!("{}{}", execution_path_as_str, "/"));

                            // Reassign the execution_path_as_str since the execution_path got changed
                            execution_path_as_str =
                                execution_path.to_str().expect("Unable to cast CStr to str");
                        }

                        path_buf = PathBuf::from(format!(
                            "{}{}",
                            execution_path_as_str,
                            path_buf.to_str().expect("Unable to cast CStr to str")
                        ));

                        if !path_buf.exists() || !path_buf.is_file() {
                            path_buf = PathBuf::from(original_path_buf);

                            continue;
                        }
                    }
                    None => {
                        return load_on_error(format!(
                            "Rs_loader was unable to find '{}' in the list of execution_paths.",
                            original_path_buf
                                .to_str()
                                .expect("Unable to cast CStr to str")
                        ))
                    }
                };

                if path_buf.exists() && path_buf.is_file() {
                    break;
                }
            }
        }
        if !path_buf.exists() || !path_buf.is_file() {
            return load_on_error(format!(
                "The file or path '{}' does not exist.",
                path_buf.to_str().expect("Unable to cast CStr to str")
            ));
        }

        match on_path_buf(path_buf, load_on_error) {
            Ok(instance) => handle_shared_object.push(instance),
            Err(error) => return error,
        };
    }

    Box::into_raw(handle_shared_object) as *mut c_void
}
