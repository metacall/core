use compiler::api;
use compiler::file::FileRegistration;
use compiler::memory::MemoryRegistration;
use compiler::package::PackageRegistration;

use std::ffi::CStr;
use std::fmt::Display;
use std::os::raw::{c_char, c_void};
use std::path::PathBuf;

#[derive(Debug)]
pub enum LoadingMethod {
    File(FileRegistration),
    Package(PackageRegistration),
    Memory(MemoryRegistration),
}

impl LoadingMethod {
    pub fn consume_dlib(self) -> Result<compiler::DynlinkLibrary, String> {
        match self {
            Self::File(FileRegistration { mut dynlink, .. }) => match dynlink {
                Some(_) => {
                    let dl = dynlink.take();
                    Ok(dl.expect("Unexpected: Dynlink library is None"))
                }
                None => Err(String::from("consume_dlib was called more than once")),
            },
            Self::Package(PackageRegistration { mut dynlink, .. }) => match dynlink {
                Some(_) => {
                    let dl = dynlink.take();
                    Ok(dl.expect("Unexpected: Dynlink library is None"))
                }
                None => Err(String::from("consume_dlib was called more than once")),
            },
            Self::Memory(MemoryRegistration { mut dynlink, .. }) => match dynlink {
                Some(_) => {
                    let dl = dynlink.take();
                    Ok(dl.expect("Unexpected: Dynlink library is None"))
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
    std::ptr::null_mut::<c_void>()
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
    if loader_impl.is_null() || loadable_path.is_null() {
        eprintln!("rs_loader load: received null pointer for loader_impl or loadable_path");
        return std::ptr::null_mut();
    }

    let loader_lifecycle_state = unsafe {
        match api::get_loader_lifecycle_state(loader_impl).as_mut() {
            Some(state) => state,
            None => {
                eprintln!("rs_loader load: unable to get lifecycle state");
                return std::ptr::null_mut();
            }
        }
    };
    let mut execution_paths_iterator = loader_lifecycle_state.execution_paths.iter();

    let mut path: *const c_char;

    let mut handle_shared_object = Box::new(Vec::new());

    for i in 0..size {
        if path_is_vector {
            path = loadable_path.wrapping_add(i) as *const c_char
        } else {
            path = loadable_path as *const i8;
        }

        let path_slice = match unsafe { CStr::from_ptr(path) }.to_str() {
            Ok(s) => s,
            Err(_) => {
                return load_on_error(String::from(
                    "rs_loader load: path is not valid UTF-8",
                ));
            }
        };
        let mut path_buf = PathBuf::from(path_slice);

        if !path_buf.is_absolute() {
            loop {
                let execution_path_current_iteration = execution_paths_iterator.next();
                let original_path_buf = path_buf.clone();

                match execution_path_current_iteration {
                    Some(original_execution_path) => {
                        let mut execution_path = original_execution_path.clone();
                        let mut execution_path_as_str =
                            execution_path.to_str().unwrap_or("<invalid path>");

                        if !execution_path_as_str.ends_with('/') {
                            execution_path =
                                PathBuf::from(format!("{}{}", execution_path_as_str, "/"));

                            // Reassign the execution_path_as_str since the execution_path got changed
                            execution_path_as_str =
                                execution_path.to_str().unwrap_or("<invalid path>");
                        }

                        path_buf = PathBuf::from(format!(
                            "{}{}",
                            execution_path_as_str,
                            path_buf.to_str().unwrap_or("<invalid path>")
                        ));

                        if !path_buf.exists() || !path_buf.is_file() {
                            path_buf = original_path_buf;

                            continue;
                        }
                    }
                    None => {
                        return load_on_error(format!(
                            "Rs_loader was unable to find '{}' in the list of execution_paths.",
                            original_path_buf
                                .to_str()
                                .unwrap_or("<invalid path>")
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
                path_buf.to_str().unwrap_or("<invalid path>")
            ));
        }

        match on_path_buf(path_buf, load_on_error) {
            Ok(instance) => handle_shared_object.push(instance),
            Err(error) => return error,
        };
    }

    Box::into_raw(handle_shared_object) as *mut c_void
}
