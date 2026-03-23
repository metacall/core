use super::loader::LoadingMethod;
use std::fmt::Display;
use std::os::raw::{c_int, c_void};

pub fn discover_on_error<T: Display>(error: T) -> c_int {
    eprintln!("Discovery error: {}", error);
    1_i32
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_discover(
    loader_impl: *mut c_void,
    handle: *mut c_void,
    ctx: *mut c_void,
) -> c_int {
    let result = std::panic::catch_unwind(|| {
    
        if loader_impl.is_null() || handle.is_null() || ctx.is_null() {
            return 1_i32;
        }


        let handle_shared_objects = &*(handle as *const Vec<LoadingMethod>);

        for handle_shared_object in handle_shared_objects.iter() {
            match handle_shared_object {
                LoadingMethod::File(file_registration) => {
                    if let Err(error) = file_registration.discover(loader_impl, ctx) {
                        return discover_on_error(error);
                    }
                }
                LoadingMethod::Memory(memory_registration) => {
                    if let Err(error) = memory_registration.discover(loader_impl, ctx) {
                        return discover_on_error(error);
                    }
                }
                LoadingMethod::Package(package_registration) => {
                    if let Err(error) = package_registration.discover(loader_impl, ctx) {
                        return discover_on_error(error);
                    }
                }
            }
        }

        0_i32 
    });

    result.unwrap_or(1_i32)
}