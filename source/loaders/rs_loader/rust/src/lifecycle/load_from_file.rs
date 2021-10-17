use crate::{bridge_api, c_char, c_int, c_void, CStr, PathBuf};

use metacall_registrator;

fn rs_loader_impl_load_from_file_on_error(error: &str) -> *mut c_void {
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
    let mut execution_paths_iterator = unsafe { (*loader_lifecycle_state).execution_paths.iter() };

    for i in 0..size {
        let path: *const c_char = paths.wrapping_offset(i as isize) as *const c_char;
        let path_slice = unsafe { CStr::from_ptr(path) }.to_str().unwrap();
        let mut path_buf = PathBuf::from(path_slice);

        if !path_buf.is_absolute() {
            let execution_path = execution_paths_iterator.next();

            match execution_path {
                Some(execution_path) => {
                    let mut execution_path = execution_path.clone();
                    let mut execution_path_as_str = execution_path.to_str().unwrap();

                    if !execution_path_as_str.ends_with("/") {
                        execution_path = PathBuf::from(format!("{}{}", execution_path_as_str, "/"));

                        //Reassign the execution_path_as_str since the execution_path got changed
                        execution_path_as_str = execution_path.to_str().unwrap();
                    }

                    path_buf = PathBuf::from(format!(
                        "{}{}",
                        execution_path_as_str,
                        path_buf.to_str().unwrap()
                    ));
                }
                None => {
                    return rs_loader_impl_load_from_file_on_error(
                        "Execution path's length is less than non-absolute path's length",
                    )
                }
            };
        }
        if !path_buf.is_file() {
            return rs_loader_impl_load_from_file_on_error("Not a valid path to a file");
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
