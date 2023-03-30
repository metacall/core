use crate::{
    bindings::{metacall_value_destroy, metacallfv_s},
    generated_bindings::metacall_value_create_function,
    parsers,
    prelude::Any,
};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Hash)]
pub struct MetacallFunction {
    func: Arc<*mut c_void>,
}
unsafe impl Send for MetacallFunction {}
unsafe impl Sync for MetacallFunction {}

impl MetacallFunction {
    pub fn new_raw(func: *mut c_void) -> Self {
        Self {
            func: Arc::new(func),
        }
    }

    pub fn new<T>(func: Box<T>) -> Self {
        Self::new_raw(unsafe { metacall_value_create_function(Box::into_raw(func) as *mut c_void) })
    }

    pub fn call<T: ToString>(&self, args: impl IntoIterator<Item = Any>) -> Any {
        let mut c_args = parsers::any_to_metacall(args);

        let ret: *mut c_void = unsafe { metacallfv_s(*self.func.clone(), c_args.as_mut_ptr(), 0) };

        let parsed_ret = parsers::metacall_to_any(ret);

        for arg in c_args {
            unsafe { metacall_value_destroy(arg) };
        }

        parsed_ret
    }

    pub fn into_raw(self) -> *mut c_void {
        *self.func
    }
}
