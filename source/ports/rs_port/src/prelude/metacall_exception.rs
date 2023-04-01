use crate::bindings::metacall_exception_type;
use std::{
    ffi::{c_char, c_void, CStr},
    ptr,
    sync::Arc,
};

#[derive(Clone, Hash)]
pub struct MetacallException {
    exception: Arc<*mut c_void>,
    exception_struct: Arc<metacall_exception_type>,
    throwable: bool,
}
unsafe impl Send for MetacallException {}
unsafe impl Sync for MetacallException {}

impl MetacallException {
    pub fn new(exception: *mut c_void, throwable: bool) -> Self {
        Self {
            exception_struct: Arc::new(unsafe {
                ptr::read(exception as *mut metacall_exception_type)
            }),
            exception: Arc::new(exception),
            throwable,
        }
    }

    fn string_convertor(string: *const c_char) -> String {
        String::from(
            unsafe { CStr::from_ptr(string as *const c_char) }
                .to_str()
                .unwrap(),
        )
    }

    pub fn get_message(&self) -> String {
        Self::string_convertor(self.exception_struct.message)
    }

    pub fn get_label(&self) -> String {
        Self::string_convertor(self.exception_struct.label)
    }

    pub fn get_stacktrace(&self) -> String {
        Self::string_convertor(self.exception_struct.stacktrace)
    }

    pub fn get_code(&self) -> i64 {
        self.exception_struct.code
    }

    pub fn is_throwable(&self) -> bool {
        self.throwable
    }

    pub fn into_raw(self) -> *mut c_void {
        *self.exception
    }
}

impl ToString for MetacallException {
    fn to_string(&self) -> String {
        let error_type = if self.throwable {
            "throwable"
        } else {
            "exception"
        };

        format!(
            "[{}(code: `{}`)]: {}",
            error_type,
            self.get_code(),
            self.get_message()
        )
    }
}
