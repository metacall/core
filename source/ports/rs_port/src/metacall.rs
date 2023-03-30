use crate::{
    bindings::{metacall_function, metacall_value_destroy, metacallfv_s},
    cstring_enum, parsers,
    prelude::{Any, MetacallError},
};
use std::ffi::c_void;

pub fn metacall<'a>(
    func: impl ToString,
    args: impl IntoIterator<Item = Any>,
) -> Result<Any, MetacallError> {
    let c_function = cstring_enum!(func, MetacallError)?;

    let c_func: *mut c_void = unsafe { metacall_function(c_function.as_ptr()) };

    if c_func.is_null() {
        return Err(MetacallError::FunctionNotFound);
    }

    let mut c_args: Vec<*mut c_void> = parsers::any_to_metacall(args);
    let ret: *mut c_void = unsafe { metacallfv_s(c_func, c_args.as_mut_ptr(), c_args.len()) };

    let parsed_ret = parsers::metacall_to_any(ret);
    unsafe { metacall_value_destroy(ret) };

    for arg in c_args {
        unsafe {
            metacall_value_destroy(arg);
        }
    }

    Ok(parsed_ret)
}
