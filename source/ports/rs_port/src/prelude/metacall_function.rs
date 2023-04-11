use super::{MetacallError, MetacallNull, MetacallObjectProtocol};
use crate::{
    bindings::{
        metacall_value_create_function, metacall_value_destroy, metacall_value_to_function,
        metacallfv_s,
    },
    parsers,
};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Debug)]
pub struct MetacallFunction {
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallFunction {}
unsafe impl Sync for MetacallFunction {}

impl MetacallFunction {
    pub fn new_raw(ptr: *mut c_void) -> Self {
        Self { ptr: Arc::new(ptr) }
    }

    pub fn new<T>(func: T) -> Self {
        unsafe {
            Self::new_raw(metacall_value_create_function(
                Box::into_raw(Box::new(func)) as *mut c_void,
            ))
        }
    }

    fn value_to_function(&self) -> *mut c_void {
        unsafe { metacall_value_to_function(*self.ptr) }
    }

    fn call_inner<T: MetacallObjectProtocol>(
        &self,
        args: impl IntoIterator<Item = T>,
    ) -> *mut c_void {
        let (mut c_args, cleanup) = parsers::metacallobj_to_raw_args(args);
        let ret: *mut c_void =
            unsafe { metacallfv_s(self.value_to_function(), c_args.as_mut_ptr(), 0) };

        cleanup();

        ret
    }
    pub fn call_untyped<T: MetacallObjectProtocol>(
        &self,
        args: impl IntoIterator<Item = T>,
    ) -> Box<dyn MetacallObjectProtocol> {
        parsers::raw_to_metacallobj_untyped(self.call_inner(args))
    }
    pub fn call<T: MetacallObjectProtocol, U: MetacallObjectProtocol>(
        &self,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_inner(args)) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    pub fn call_no_arg<T: MetacallObjectProtocol>(&self) -> Result<T, MetacallError> {
        self.call::<T, MetacallNull>([])
    }

    pub fn into_raw(self) -> *mut c_void {
        self.value_to_function()
    }
}

impl Drop for MetacallFunction {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) };
    }
}
