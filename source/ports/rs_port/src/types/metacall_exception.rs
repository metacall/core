use super::{MetaCallStringConversionError, MetaCallValue};
use crate::{
    bindings::{
        metacall_error_from_value, metacall_error_throw, metacall_exception_type,
        metacall_throwable_value, metacall_value_copy, metacall_value_destroy,
        metacall_value_to_throwable,
    },
    cast, cstring,
};
use std::{
    ffi::{c_void, CStr, CString},
    fmt::{self, Debug, Formatter},
};

unsafe impl Send for metacall_exception_type {}
unsafe impl Sync for metacall_exception_type {}

/// Represents MetaCall exception. You can create an exception with [new](#method.new).
pub struct MetaCallException {
    message: String,
    label: String,
    stacktrace: String,
    code: i64,
    leak: bool,
    value: *mut c_void,
}

unsafe impl Send for MetaCallException {}
unsafe impl Sync for MetaCallException {}

impl Clone for MetaCallException {
    fn clone(&self) -> Self {
        let value = if self.value.is_null() {
            std::ptr::null_mut()
        } else {
            let copied = unsafe { metacall_value_copy(self.value) };
            assert!(
                !copied.is_null(),
                "metacall_value_copy failed to clone MetaCallException"
            );
            copied
        };

        Self {
            message: self.message.clone(),
            label: self.label.clone(),
            stacktrace: self.stacktrace.clone(),
            code: self.code,
            leak: false,
            value,
        }
    }
}
impl Debug for MetaCallException {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCallException: {}", self)
    }
}

impl MetaCallException {
    /// Creates a new exception.
    pub fn new(
        message: impl ToString,
        label: impl ToString,
        stacktrace: impl ToString,
        code: i64,
    ) -> Result<Self, MetaCallStringConversionError> {
        let message_str = message.to_string();
        let label_str = label.to_string();
        let stacktrace_str = stacktrace.to_string();

        let c_message = cstring!(&message_str)?;
        let c_label = cstring!(&label_str)?;
        let c_stacktrace = cstring!(&stacktrace_str)?;
        let format_str = CString::new("%s").unwrap();

        let th = unsafe {
            metacall_error_throw(
                c_label.as_ptr(),
                code,
                c_stacktrace.as_ptr(),
                format_str.as_ptr(),
                c_message.as_ptr(),
            )
        };

        assert!(
            !th.is_null(),
            "metacall_error_throw failed to allocate exception throwable"
        );

        let th_obj = unsafe { metacall_value_to_throwable(th) };
        if th_obj.is_null() {
            unsafe { metacall_value_destroy(th) };
            panic!("metacall_value_to_throwable returned null");
        }

        let ex_val = unsafe { metacall_throwable_value(th_obj) };
        if ex_val.is_null() {
            unsafe { metacall_value_destroy(th) };
            panic!("metacall_throwable_value returned null");
        }

        let copied = unsafe { metacall_value_copy(ex_val) };
        unsafe { metacall_value_destroy(th) };

        assert!(
            !copied.is_null(),
            "metacall_value_copy failed to copy exception value"
        );

        Ok(Self {
            message: message_str,
            label: label_str,
            stacktrace: stacktrace_str,
            code,
            value: copied,
            leak: false,
        })
    }

    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        let mut ex_struct = metacall_exception_type {
            message: std::ptr::null(),
            label: std::ptr::null(),
            stacktrace: std::ptr::null(),
            code: 0,
        };

        if !value.is_null() {
            // metacall_error_from_value returns 0 on success, or non-zero if value is not an
            // exception or throwable. If it fails, ex_struct remains null/zero-initialized.
            let _ = unsafe { metacall_error_from_value(value, &mut ex_struct) };
        }

        let message = if ex_struct.message.is_null() {
            String::new()
        } else {
            unsafe { CStr::from_ptr(ex_struct.message) }
                .to_string_lossy()
                .into_owned()
        };
        let label = if ex_struct.label.is_null() {
            String::new()
        } else {
            unsafe { CStr::from_ptr(ex_struct.label) }
                .to_string_lossy()
                .into_owned()
        };
        let stacktrace = if ex_struct.stacktrace.is_null() {
            String::new()
        } else {
            unsafe { CStr::from_ptr(ex_struct.stacktrace) }
                .to_string_lossy()
                .into_owned()
        };

        Self {
            message,
            label,
            stacktrace,
            code: ex_struct.code,
            leak: false,
            value,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        let mut s = Self::new_raw(value);
        s.leak = true;
        s
    }

    /// Gets the exception message.
    pub fn get_message(&self) -> String {
        self.message.clone()
    }
    /// Gets the exception label.
    pub fn get_label(&self) -> String {
        self.label.clone()
    }
    /// Gets the exception stacktrace.
    pub fn get_stacktrace(&self) -> String {
        self.stacktrace.clone()
    }
    /// Gets the exception code.
    pub fn get_code(&self) -> i64 {
        self.code
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

#[derive(Debug, Clone)]
/// Different types of Throwable value.
pub enum MetaCallThrowableValue<T: MetaCallValue> {
    /// Exception.
    Exception(MetaCallException),
    /// Other types.
    Other(Box<dyn MetaCallValue>),
    /// Specified.
    Specified(T),
}

/// Represents MetaCall throwable. Keep in mind that it's not supported to pass a throwable as an argument.
pub struct MetaCallThrowable {
    leak: bool,
    value_ptr: *mut c_void,
    value: *mut c_void,
}
unsafe impl Send for MetaCallThrowable {}
unsafe impl Sync for MetaCallThrowable {}
impl Clone for MetaCallThrowable {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            value: self.value,
            value_ptr: self.value_ptr,
        }
    }
}
impl Debug for MetaCallThrowable {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCallThrowable: {}", self)
    }
}

impl MetaCallThrowable {
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

    /// Gets the throwable value without type casting([MetaCallValue](MetaCallValue)).
    pub fn get_value_untyped(&self) -> Box<dyn MetaCallValue> {
        match cast::raw_to_metacallobj::<MetaCallException>(self.value) {
            Ok(mut value) => {
                value.leak = true;

                cast::metacall_box(value)
            }
            Err(original) => original,
        }
    }
    /// Gets the throwable value.
    pub fn get_value<T: MetaCallValue>(&self) -> Result<T, Box<dyn MetaCallValue>> {
        match self.get_value_untyped().downcast::<T>() {
            Ok(value) => Ok(value),
            Err(original) => Err(original),
        }
    }

    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        // It's not implemented in any loader as the time of writing this block of code.
        // Feel free to implement as any loader adopted accepting Throwable as an argument.

        panic!("Passing MetaCallThrowable as an argument is not supported!");
    }
}

impl fmt::Display for MetaCallException {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "[Exception(code: `{}`)]: {}",
            self.get_code(),
            self.get_message()
        )
    }
}

impl fmt::Display for MetaCallThrowable {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let throwable_value = self.get_value_untyped();
        write!(f, "[Throwable]: {:#?}", throwable_value)
    }
}

impl Drop for MetaCallException {
    fn drop(&mut self) {
        if !self.leak && !self.value.is_null() {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
impl Drop for MetaCallThrowable {
    fn drop(&mut self) {
        unsafe {
            if !self.leak {
                metacall_value_destroy(self.value_ptr);
            }
        }
    }
}
