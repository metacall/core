use super::loader::LoadingMethod;
use crate::{c_int, c_void};

use core::fmt::Display;

pub fn discover_on_error<T: Display>(error: T) -> c_int {
    eprintln!("{}", error);

    1 as c_int
}

#[no_mangle]
pub extern "C" fn rs_loader_impl_discover(
    loader_impl: *mut c_void,
    handle: *mut c_void,
    ctx: *mut c_void,
) -> c_int {
    let handle_shared_objects = unsafe { Box::from_raw(handle as *mut Vec<LoadingMethod>) };

    for handle_shared_object in handle_shared_objects.iter() {
        match handle_shared_object.clone() {
            LoadingMethod::File(file_registration) => {
                if let Err(error) = file_registration.discover(loader_impl, ctx) {
                    return discover_on_error(error);
                }
            } // LoadingMethod::Package(package_registration) => {
              //     if let Err(error) = package_registration.discover(ctx) {
              //         return discover_on_error(error);
              //     }
              // }
        }
    }

    0 as c_int
}
