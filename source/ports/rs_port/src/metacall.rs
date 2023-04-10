use crate::{
    bindings::{metacall_function, metacallfv_s},
    cstring_enum, parsers,
    prelude::{MetacallError, MetacallNull, MetacallObjectProtocol},
};
use std::ffi::c_void;

fn metacall_inner(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetacallObjectProtocol>,
) -> Result<*mut c_void, MetacallError> {
    let c_function = cstring_enum!(func, MetacallError)?;
    let c_func = unsafe { metacall_function(c_function.as_ptr()) };

    if c_func.is_null() {
        return Err(MetacallError::FunctionNotFound);
    }

    let (mut c_args, cleanup) = parsers::metacallobj_to_raw_args(args);
    let args_length = c_args.len();

    let ret = unsafe { metacallfv_s(c_func, c_args.as_mut_ptr(), args_length) };

    cleanup();

    Ok(ret)
}
pub fn metacall_untyped(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetacallObjectProtocol>,
) -> Result<Box<dyn MetacallObjectProtocol>, MetacallError> {
    Ok(parsers::raw_to_metacallobj_untyped(metacall_inner(
        func, args,
    )?))
}
pub fn metacall<T: MetacallObjectProtocol>(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetacallObjectProtocol>,
) -> Result<T, MetacallError> {
    match parsers::raw_to_metacallobj::<T>(metacall_inner(func, args)?) {
        Ok(ret) => Ok(ret),
        Err(original) => Err(MetacallError::FailedCasting(original)),
    }
}
pub fn metacall_no_arg<T: MetacallObjectProtocol>(func: impl ToString) -> Result<T, MetacallError> {
    metacall::<T>(func, [] as [MetacallNull; 0])
}
