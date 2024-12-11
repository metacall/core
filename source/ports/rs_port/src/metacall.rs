use crate::{
    bindings::{metacall_function, metacall_value_destroy, metacallfv_s},
    cstring_enum, parsers,
    types::{MetaCallError, MetaCallNull, MetaCallValue},
};
use std::ffi::c_void;

// Used for documentation.
#[allow(unused_imports)]
use crate::match_metacall_value;

fn metacall_inner(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetaCallValue>,
) -> Result<*mut c_void, MetaCallError> {
    let c_function = cstring_enum!(func, MetaCallError)?;
    let c_func = unsafe { metacall_function(c_function.as_ptr()) };

    if c_func.is_null() {
        return Err(MetaCallError::FunctionNotFound);
    }

    let mut c_args = parsers::metacallobj_to_raw_args(args);
    let args_length = c_args.len();

    let ret = unsafe { metacallfv_s(c_func, c_args.as_mut_ptr(), args_length) };

    for c_arg in c_args {
        unsafe { metacall_value_destroy(c_arg) };
    }

    Ok(ret)
}
/// Calls a function same as [metacall](metacall) but returns a trait object
/// of [MetaCallValue](MetaCallValue). This is useful when you don't know the return
/// type of that function or the function may return multiple types. Checkout
/// [match_metacall_value](match_metacall_value) for unwrapping the inner value. For example: ...
/// ```
/// let sum = metacall::metacall_untyped("sum", [1, 2]).unwrap();
/// ```
pub fn metacall_untyped(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetaCallValue>,
) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
    Ok(parsers::raw_to_metacallobj_untyped(metacall_inner(
        func, args,
    )?))
}
/// Calls a function same as [metacall_untyped](metacall_untyped) without passing any arguments. For example: ...
/// ```
/// let greet = metacall::metacall_untyped_no_arg("sum").unwrap();
/// ```
pub fn metacall_untyped_no_arg(
    func: impl ToString,
) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
    metacall_untyped(func, [] as [MetaCallNull; 0])
}
/// Calls a function with arguments. The generic parameter is the return type of the function
/// you're calling. Checkout [MetaCallValue](MetaCallValue) for possible types.
/// For example: ...
/// ```
/// let sum = metacall::metacall::<i32>("sum", [1, 2]).unwrap();
/// ```
pub fn metacall<T: MetaCallValue>(
    func: impl ToString,
    args: impl IntoIterator<Item = impl MetaCallValue>,
) -> Result<T, MetaCallError> {
    match parsers::raw_to_metacallobj::<T>(metacall_inner(func, args)?) {
        Ok(ret) => Ok(ret),
        Err(original) => Err(MetaCallError::FailedCasting(original)),
    }
}
/// Calls a function same as [metacall](metacall) without passing any arguments. For example: ...
/// ```
/// let greet = metacall::metacall_no_arg::<String>("greet").unwrap();
/// ```
pub fn metacall_no_arg<T: MetaCallValue>(func: impl ToString) -> Result<T, MetaCallError> {
    metacall::<T>(func, [] as [MetaCallNull; 0])
}
