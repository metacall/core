use super::{MetacallError, MetacallNull, MetacallValue};
use crate::{
    bindings::{metacall_value_destroy, metacall_value_to_function, metacallfv_s},
    parsers,
};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

/// Represents Metacall function.
pub struct MetacallFunction {
    leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetacallFunction {}
unsafe impl Sync for MetacallFunction {}
impl Clone for MetacallFunction {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetacallFunction {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetacallFunction {{ ... }}")
    }
}

impl MetacallFunction {
    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        Self { leak: false, value }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        Self { leak: true, value }
    }

    fn value_to_function(&self) -> *mut c_void {
        unsafe { metacall_value_to_function(self.value) }
    }

    fn call_inner<T: MetacallValue>(&self, args: impl IntoIterator<Item = T>) -> *mut c_void {
        let mut c_args = parsers::metacallobj_to_raw_args(args);
        let ret: *mut c_void =
            unsafe { metacallfv_s(self.value_to_function(), c_args.as_mut_ptr(), 0) };

        for c_arg in c_args {
            unsafe { metacall_value_destroy(c_arg) };
        }

        ret
    }
    /// Calls the function with arguments and witout type casting([MetacallValue](MetacallValue)).
    pub fn call_untyped<T: MetacallValue>(
        &self,
        args: impl IntoIterator<Item = T>,
    ) -> Box<dyn MetacallValue> {
        parsers::raw_to_metacallobj_untyped(self.call_inner(args))
    }
    /// Calls the function without passing arguments and witout type
    /// casting([MetacallValue](MetacallValue)).
    pub fn call_untyped_no_arg<T: MetacallValue>(&self) -> Box<dyn MetacallValue> {
        parsers::raw_to_metacallobj_untyped(self.call_inner([] as [MetacallNull; 0]))
    }
    /// Calls the function with arguments.
    pub fn call<T: MetacallValue, U: MetacallValue>(
        &self,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_inner(args)) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    /// Calls the function without arguments.
    pub fn call_no_arg<T: MetacallValue>(&self) -> Result<T, MetacallError> {
        self.call::<T, MetacallNull>([])
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

impl Drop for MetacallFunction {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
