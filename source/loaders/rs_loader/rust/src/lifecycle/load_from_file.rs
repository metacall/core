use crate::{bridge_api, c_char, c_int, c_void, CStr, PathBuf};

use std::fmt::Display;

use metacall_registrator;

fn rs_loader_impl_load_from_file_on_error<T: Display>(error: T) -> *mut c_void {
    eprintln!("{}", error);

    0 as c_int as *mut c_void
}

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_file(
    loader_impl: *mut c_void,
    paths: *const *mut c_char,
    size: usize,
) -> *mut c_void {
    let loader_lifecycle_state = bridge_api::get_loader_lifecycle_state(loader_impl);

    for i in 0..size {
        let path: *const c_char = paths.wrapping_offset(i as isize) as *const c_char;
        let path_slice = unsafe { CStr::from_ptr(path) }.to_str().unwrap();
        let mut path_buf = PathBuf::from(path_slice);

        let mut execution_paths_iterator =
            unsafe { (*loader_lifecycle_state).execution_paths.iter() };

        if !path_buf.is_absolute() {
            loop {
                let execution_path_current_iteration = execution_paths_iterator.next();
                let original_path_buf = path_buf.clone();

                match execution_path_current_iteration {
                    Some(original_execution_path) => {
                        let mut execution_path = original_execution_path.clone();
                        let mut execution_path_as_str = execution_path.to_str().unwrap();

                        if !execution_path_as_str.ends_with("/") {
                            execution_path =
                                PathBuf::from(format!("{}{}", execution_path_as_str, "/"));

                            //Reassign the execution_path_as_str since the execution_path got changed
                            execution_path_as_str = execution_path.to_str().unwrap();
                        }

                        path_buf = PathBuf::from(format!(
                            "{}{}",
                            execution_path_as_str,
                            path_buf.to_str().unwrap()
                        ));

                        if !path_buf.exists() || !path_buf.is_file() {
                            path_buf = PathBuf::from(original_path_buf);

                            continue;
                        }
                    }
                    None => {
                        return rs_loader_impl_load_from_file_on_error(format!(
                        "Rs_loader was unable to find any suitable execution_path for the path: {}",
                        original_path_buf.to_str().unwrap()
                    ))
                    }
                };

                if path_buf.exists() && path_buf.is_file() {
                    break;
                }
            }
        }
        if !path_buf.is_file() {
            return rs_loader_impl_load_from_file_on_error(format!(
                "Not a valid absolute path to a file, {}",
                path_buf.to_str().unwrap()
            ));
        }

        let cargo_cdylib_project =
            match metacall_registrator::CargoCdylibProject::new(path_buf, loader_impl) {
                Ok(cargo_cdylib_project) => cargo_cdylib_project,
                Err(error) => return rs_loader_impl_load_from_file_on_error(error),
            };

        match cargo_cdylib_project.register_the_project_in_metacall() {
            Ok(_) => (),
            Err(error) => return rs_loader_impl_load_from_file_on_error(error),
        };
    }

    1 as c_int as *mut c_void
}
