use super::{MetaCallNull, MetaCallStringConversionError, MetaCallValue};
use crate::{
    bindings::{
        metacall_exception_type, metacall_throwable_value, metacall_value_copy,
        metacall_value_create_exception, metacall_value_destroy, metacall_value_id,
        metacall_value_to_exception, metacall_value_to_throwable,
    },
    cast, cstring,
};
use std::{
    ffi::{c_char, c_void, CStr},
    fmt::{self, Debug, Formatter},
    rc::Rc,
    sync::Arc,
};

unsafe impl Send for metacall_exception_type {}
unsafe impl Sync for metacall_exception_type {}

/// Represents MetaCall exception. You can create an exception with [new](#method.new).
pub struct MetaCallException {
    exception_struct: Arc<metacall_exception_type>,
    leak: bool,
    value: *mut c_void,
}

unsafe impl Send for MetaCallException {}
unsafe impl Sync for MetaCallException {}

impl Clone for MetaCallException {
    fn clone(&self) -> Self {
        Self {
            exception_struct: self.exception_struct.clone(),
            leak: true,
            value: self.value,
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
pub enum MetaCallThrowableValue<T: MetaCallValue> {
    /// Exception.
    Exception(MetaCallException),
    /// Other types.
    Other(Box<dyn MetaCallValue>),
    /// Specified.
    Specified(T),
}

#[derive(Debug)]
struct MetaCallThrowableInner {
    leak: bool,
    value_ptr: *mut c_void,
    value: *mut c_void,
}

impl Drop for MetaCallThrowableInner {
    fn drop(&mut self) {
        if !self.leak && !self.value_ptr.is_null() {
            unsafe {
                metacall_value_destroy(self.value_ptr);
            }
        }
    }
}

/// Represents MetaCall throwable. Keep in mind that it's not supported to pass a throwable as an argument.
pub struct MetaCallThrowable {
    inner: Rc<MetaCallThrowableInner>,
}

impl Clone for MetaCallThrowable {
    fn clone(&self) -> Self {
        Self {
            inner: Rc::clone(&self.inner),
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
        let throwable_value = if !value_ptr.is_null() {
            unsafe {
                let th = metacall_value_to_throwable(value_ptr);
                if !th.is_null() {
                    metacall_throwable_value(th)
                } else {
                    std::ptr::null_mut()
                }
            }
        } else {
            std::ptr::null_mut()
        };
        Self {
            inner: Rc::new(MetaCallThrowableInner {
                leak: false,
                value_ptr,
                value: throwable_value,
            }),
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value_ptr: *mut c_void) -> Self {
        let throwable_value = if !value_ptr.is_null() {
            unsafe {
                let th = metacall_value_to_throwable(value_ptr);
                if !th.is_null() {
                    metacall_throwable_value(th)
                } else {
                    std::ptr::null_mut()
                }
            }
        } else {
            std::ptr::null_mut()
        };
        Self {
            inner: Rc::new(MetaCallThrowableInner {
                leak: true,
                value_ptr,
                value: throwable_value,
            }),
        }
    }

    /// Gets the throwable value without type casting([MetaCallValue](MetaCallValue)).
    pub fn get_value_untyped(&self) -> Box<dyn MetaCallValue> {
        let inner_val = self.inner.value;
        if inner_val.is_null() {
            return cast::metacall_box(MetaCallNull());
        }

        let type_id = unsafe { metacall_value_id(inner_val) };
        if type_id == metacall_value_id::METACALL_THROWABLE {
            // Nested TYPE_THROWABLE cannot be safely copied due to upstream C reflect_value_type defect
            return cast::metacall_box(MetaCallNull());
        }

        let copied = unsafe { metacall_value_copy(inner_val) };
        if copied.is_null() {
            return cast::metacall_box(MetaCallNull());
        }

        cast::raw_to_metacallobj_untyped(copied)
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
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
