use super::{MetacallStringConversionError, MetacallValue};
use crate::{
    bindings::{
        metacall_exception_type, metacall_throwable_value, metacall_value_create_exception,
        metacall_value_destroy, metacall_value_to_exception, metacall_value_to_throwable,
    },
    cstring, helpers, parsers,
};
use std::{
    ffi::{c_char, c_void, CStr},
    fmt::{self, Debug, Formatter},
    sync::Arc,
};

/// Represents Metacall exception. You can create an exception with [new](#method.new).
pub struct MetacallException {
    exception_struct: Arc<metacall_exception_type>,
    leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetacallException {}
unsafe impl Sync for MetacallException {}
impl Clone for MetacallException {
    fn clone(&self) -> Self {
        Self {
            exception_struct: self.exception_struct.clone(),
            leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetacallException {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "MetacallException {}",
            format!("{{ {} }}", self.to_string())
        )
    }
}

impl MetacallException {
    /// Creates a new exception.
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
            value: exception_ptr,
            leak: false,
        })
    }

    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        let exception = unsafe { metacall_value_to_exception(value) };
        Self {
            exception_struct: Arc::new(unsafe { *(exception as *mut metacall_exception_type) }),
            leak: false,
            value,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        let exception = unsafe { metacall_value_to_exception(value) };
        Self {
            exception_struct: Arc::new(unsafe { *(exception as *mut metacall_exception_type) }),
            leak: true,
            value,
        }
    }

    fn string_convertor(string: *const c_char) -> String {
        String::from(unsafe { CStr::from_ptr(string) }.to_str().unwrap())
    }

    /// Gets the exception message.
    pub fn get_message(&self) -> String {
        Self::string_convertor(self.exception_struct.message)
    }
    /// Gets the exception label.
    pub fn get_label(&self) -> String {
        Self::string_convertor(self.exception_struct.label)
    }
    /// Gets the exception stacktrace.
    pub fn get_stacktrace(&self) -> String {
        Self::string_convertor(self.exception_struct.stacktrace)
    }
    /// Gets the exception code.
    pub fn get_code(&self) -> i64 {
        self.exception_struct.code
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

#[derive(Debug, Clone)]
/// Different types of Throwable value.
pub enum MetacallThrowableValue<T: MetacallValue> {
    /// Exception.
    Exception(MetacallException),
    /// Other types.
    Other(Box<dyn MetacallValue>),
    /// Specified.
    Specified(T),
}

/// Represents Metacall throwable. Keep in mind that it's not supported to pass a throwable as an argument.
pub struct MetacallThrowable {
    leak: bool,
    value_ptr: *mut c_void,
    value: *mut c_void,
}
unsafe impl Send for MetacallThrowable {}
unsafe impl Sync for MetacallThrowable {}
impl Clone for MetacallThrowable {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            value: self.value,
            value_ptr: self.value_ptr,
        }
    }
}
impl Debug for MetacallThrowable {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "MetacallThrowable {}",
            format!("{{ {} }}", self.to_string())
        )
    }
}

impl MetacallThrowable {
    #[doc(hidden)]
    pub fn new_raw(value_ptr: *mut c_void) -> Self {
        let throwable_value =
            unsafe { metacall_throwable_value(metacall_value_to_throwable(value_ptr)) };
        Self {
            leak: false,
            value: throwable_value,
            value_ptr,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value_ptr: *mut c_void) -> Self {
        let throwable_value =
            unsafe { metacall_throwable_value(metacall_value_to_throwable(value_ptr)) };
        Self {
            leak: true,
            value: throwable_value,
            value_ptr,
        }
    }

    /// Gets the throwable value without type casting([MetacallValue](MetacallValue)).
    pub fn get_value_untyped(&self) -> Box<dyn MetacallValue> {
        match parsers::raw_to_metacallobj::<MetacallException>(self.value) {
            Ok(mut value) => {
                value.leak = true;

                helpers::metacall_implementer_to_traitobj(value)
            }
            Err(original) => original,
        }
    }
    /// Gets the throwable value.
    pub fn get_value<T: MetacallValue>(&self) -> Result<T, Box<dyn MetacallValue>> {
        match self.get_value_untyped().downcast::<T>() {
            Ok(value) => Ok(value),
            Err(original) => Err(original),
        }
    }

    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        // It's not implemented in any loader as the time of writing this block of code.
        // Feel free to implement as any loader adopted accepting Throwable as an argument.

        panic!("Passing MetacallThrowable as an argument is not supported!");
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
        format!("[Throwable]: {:#?}", throwable_value)
    }
}

impl Drop for MetacallException {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
impl Drop for MetacallThrowable {
    fn drop(&mut self) {
        unsafe {
            if !self.leak {
                metacall_value_destroy(self.value_ptr);
            }
        }
    }
}
