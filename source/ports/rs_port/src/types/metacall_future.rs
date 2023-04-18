use super::{MetacallNull, MetacallValue};
use crate::{
    bindings::{metacall_await_future, metacall_value_destroy, metacall_value_to_future},
    helpers, parsers,
};
use std::{ffi::c_void, ptr};

/// Function pointer type used for resolving/rejecting Metacall futures. The first argument is the result
/// and the second argument is the data that you may want to access when the function gets called.
/// Checkout [MetacallFuture resolve](MetacallFuture#method.then) or
/// [MetacallFuture reject](MetacallFuture#method.catch) for usage.
pub type MetacallFutureHandler = fn(Box<dyn MetacallValue>, Box<dyn MetacallValue>);

#[derive(Debug)]
/// Represents MetacallFuture. Usage example: ...
/// ```
/// use metacall::{MetacallValue, MetacallFuture, metacall};
///
/// fn resolve(result: impl MetacallValue, data: impl MetacallValue) {
///     println!("Resolve:: result: {:#?}, data: {:#?}", result, data);
/// }
///
/// fn reject(result: impl MetacallValue, data: impl MetacallValue) {
///     println!("Reject:: result: {:#?}, data: {:#?}", result, data);
/// }
///
/// let future = metacall::<MetacallFuture>("async_function", [1]).unwrap();
/// future.then(resolve).catch(reject).await_fut();
/// ```
#[repr(C)]
pub struct MetacallFuture {
    data: *mut dyn MetacallValue,
    leak: bool,
    reject: Option<MetacallFutureHandler>,
    resolve: Option<MetacallFutureHandler>,
    value: *mut c_void,
}
unsafe impl Send for MetacallFuture {}
unsafe impl Sync for MetacallFuture {}
impl Clone for MetacallFuture {
    fn clone(&self) -> Self {
        Self {
            data: self.data,
            leak: true,
            reject: self.reject,
            resolve: self.resolve,
            value: self.value,
        }
    }
}

unsafe extern "C" fn resolver(resolve_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let self_struct = *Box::from_raw(upper_data as *mut MetacallFuture);
    let user_data = Box::from_raw(self_struct.data);

    (self_struct.resolve.unwrap())(parsers::raw_to_metacallobj_untyped(resolve_data), user_data);

    ptr::null_mut()
}
unsafe extern "C" fn rejecter(reject_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let self_struct = *Box::from_raw(upper_data as *mut MetacallFuture);
    let user_data = Box::from_raw(self_struct.data);

    (self_struct.reject.unwrap())(parsers::raw_to_metacallobj_untyped(reject_data), user_data);

    if !self_struct.leak {
        unsafe { metacall_value_destroy(self_struct.value) };
    }

    ptr::null_mut()
}

impl MetacallFuture {
    fn create_null_data() -> *mut dyn MetacallValue {
        Box::into_raw(helpers::metacall_implementer_to_traitobj(MetacallNull()))
    }

    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        Self {
            data: Self::create_null_data(),
            leak: false,
            reject: None,
            resolve: None,
            value,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        Self {
            data: Self::create_null_data(),
            leak: true,
            reject: None,
            resolve: None,
            value,
        }
    }

    /// Adds a resolve callback.
    pub fn then(mut self, resolve: MetacallFutureHandler) -> Self {
        self.resolve = Some(resolve);

        self
    }

    /// Adds a reject callback.
    pub fn catch(mut self, reject: MetacallFutureHandler) -> Self {
        self.reject = Some(reject);

        self
    }

    /// Adds data.
    pub fn data(mut self, data: impl MetacallValue) -> Self {
        unsafe { drop(Box::from_raw(self.data)) };

        self.data = Box::into_raw(Box::new(data) as Box<dyn MetacallValue>);

        self
    }

    /// Awaits the future.
    pub fn await_fut(self) {
        let resolve_is_some = self.resolve.is_some();
        let reject_is_some = self.reject.is_some();
        let value = self.value;

        unsafe {
            metacall_value_destroy(metacall_await_future(
                metacall_value_to_future(value),
                if resolve_is_some {
                    Some(resolver)
                } else {
                    None
                },
                if reject_is_some { Some(rejecter) } else { None },
                Box::into_raw(Box::new(self)) as *mut c_void,
            ))
        };
    }

    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        self.value
    }
}

// impl Drop for MetacallFuture {
//     fn drop(&mut self) {
//         if !self.leak {
//             unsafe { metacall_value_destroy(self.value) };
//         }
//     }
// }
