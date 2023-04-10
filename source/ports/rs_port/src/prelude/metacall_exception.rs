use super::{MetacallObjectProtocol, MetacallStringConversionError};
use crate::{
    bindings::{
        metacall_exception_type, metacall_throwable_value, metacall_value_create_exception,
        metacall_value_destroy, metacall_value_to_exception, metacall_value_to_throwable,
    },
    cstring, match_object_protocol, parsers,
};
use std::{
    ffi::{c_char, c_void, CStr},
    sync::Arc,
};

#[derive(Clone, Debug)]
pub struct MetacallException {
    exception_struct: Arc<metacall_exception_type>,
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallException {}
unsafe impl Sync for MetacallException {}

impl MetacallException {
    pub fn new(
        message: impl ToString,
        label: impl ToString,
        stacktrace: impl ToString,
        code: i64,
    ) -> Result<Self, MetacallStringConversionError> {
        let message = cstring!(message)?.into_raw();
        let label = cstring!(label)?.into_raw();
        let stacktrace = cstring!(stacktrace)?.into_raw();
        let mut exception_struct = metacall_exception_type {
            message: message.cast(),
            label: label.cast(),
            stacktrace: stacktrace.cast(),
            code,
        };
        let exception_ptr = unsafe {
            metacall_value_create_exception(&mut exception_struct as *mut _ as *mut c_void)
        };

        Ok(Self {
            exception_struct: Arc::new(exception_struct),
            ptr: Arc::new(exception_ptr),
        })
    }

    pub fn from(ptr: *mut c_void) -> Self {
        let exception = unsafe { metacall_value_to_exception(ptr) };
        Self {
            exception_struct: Arc::new(unsafe { *(exception as *mut metacall_exception_type) }),
            ptr: Arc::new(ptr),
        }
    }

    fn string_convertor(string: *const c_char) -> String {
        String::from(unsafe { CStr::from_ptr(string) }.to_str().unwrap())
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

    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_to_exception(*self.ptr) }
    }
}

#[derive(Clone, Debug)]
pub enum MetacallThrowableValue<T: MetacallObjectProtocol> {
    Exception(MetacallException),
    Other(Box<dyn MetacallObjectProtocol>),
    Specified(T),
}

#[derive(Clone, Debug)]
pub struct MetacallThrowable {
    value: Arc<*mut c_void>,
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallThrowable {}
unsafe impl Sync for MetacallThrowable {}

impl MetacallThrowable {
    pub fn new(ptr: *mut c_void) -> Self {
        let throwable_value = unsafe { metacall_throwable_value(metacall_value_to_throwable(ptr)) };
        Self {
            ptr: Arc::new(ptr),
            value: Arc::new(throwable_value),
        }
    }

    pub fn get_value_untyped(&self) -> Box<dyn MetacallObjectProtocol> {
        match parsers::raw_to_metacallobj::<MetacallException>(*self.value) {
            Ok(value) => parsers::implementer_to_traitobj(value),
            Err(original) => original,
        }
    }
    pub fn get_value<T: MetacallObjectProtocol>(
        &self,
    ) -> Result<T, Box<dyn MetacallObjectProtocol>> {
        match self.get_value_untyped().downcast::<T>() {
            Ok(value) => Ok(value),
            Err(original) => Err(original),
        }
    }

    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_to_throwable(*self.ptr) }
    }
}

impl ToString for MetacallException {
    fn to_string(&self) -> String {
        format!(
            "[Exception(code: `{}`)]: {}",
            self.get_code(),
            self.get_message()
        )
    }
}
impl ToString for MetacallThrowable {
    fn to_string(&self) -> String {
        let throwable_value = self.get_value_untyped();
        format!(
            "[Throwable]: {}",
            match_object_protocol!(throwable_value, {
                exception: MetacallException => exception.to_string(),
                _ => format!("{:#?}", throwable_value)
            })
        )
    }
}

impl Drop for MetacallException {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) }
    }
}
impl Drop for MetacallThrowable {
    fn drop(&mut self) {
        unsafe {
            metacall_value_destroy(*self.ptr);
        }
    }
}
