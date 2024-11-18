use super::{MetacallNull, MetacallValue};
use crate::{
    bindings::{metacall_await_future, metacall_value_destroy, metacall_value_to_future},
    parsers,
};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
    ptr,
};

/// Function pointer type used for resolving/rejecting Metacall futures. The first argument is the result
/// and the second argument is the data that you may want to access when the function gets called.
/// Checkout [MetacallFuture resolve](MetacallFuture#method.then) or
/// [MetacallFuture reject](MetacallFuture#method.catch) for usage.
pub type MetacallFutureHandler<T> = fn(Box<dyn MetacallValue>, T) -> Box<dyn MetacallValue>;

/// Represents MetacallFuture. Keep in mind that it's not supported to pass a future as an argument.
///
/// ## **Usage example:**
/// ```rust
/// use metacall::{loaders, metacall, switch, MetacallFuture, MetacallValue};
/// let script = r#"
/// function doubleValueAfterTime(value, delay) {
///     return new Promise((resolve, reject) => {
///         setTimeout(() => {
///             if (typeof value === 'number') {
///                 resolve(value * 2); // Resolves if the value is a number
///             } else {
///                 reject('Error: The provided value is not a number.'); // Rejects if the value is not a number
///             }
///         }, delay);
///     });
/// }
///
/// module.exports = {
///     doubleValueAfterTime
/// }
/// "#;
///
/// let _metacall = switch::initialize().unwrap();
/// loaders::from_memory("node", script).unwrap();
///
/// fn resolve<T: PartialEq<i16> + Debug>(result: Box<dyn MetacallValue>, data: T) {
///     let result = result.downcast::<f64>().unwrap();
///
///     assert_eq!(
///         result, 2.0,
///         "the result should be double of the passed value"
///     );
///     assert_eq!(data, 100, "data should be passed without change");
/// }
/// fn reject<T>(_: Box<dyn MetacallValue>, _: T) {
///     panic!("It shouldnt be rejected");
/// }
/// let future = metacall::<MetacallFuture<i16>>("doubleValueAfterTime", [1, 2000]).unwrap();
/// future
///     .then(resolve::<i16>)
///     .catch(reject::<i16>)
///     .data(100)
///     .await_fut();
/// ```
#[repr(C)]
pub struct MetacallFuture<T> {
    data: *mut Option<T>,
    leak: bool,
    reject: Option<MetacallFutureHandler<T>>,
    resolve: Option<MetacallFutureHandler<T>>,
    value: *mut c_void,
}
unsafe impl<T> Send for MetacallFuture<T> {}
unsafe impl<T> Sync for MetacallFuture<T> {}
impl<T> Clone for MetacallFuture<T> {
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
impl<T: Debug> Debug for MetacallFuture<T> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let boxed_data = unsafe { Box::from_raw(self.data) };

        let data = match Box::into(boxed_data) {
            Some(data) => Some(format!("{data:#?}")),
            None => None,
        };

        let resolve = if self.resolve.is_none() {
            "None"
        } else {
            "Some"
        };

        let reject = if self.reject.is_none() {
            "None"
        } else {
            "Some"
        };

        f.debug_struct("MetacallFuture")
            .field("data", &data)
            .field("resolve", &resolve)
            .field("reject", &reject)
            .finish()
    }
}

type MetacallFutureFFIData<T> = (
    // Resolve
    Option<MetacallFutureHandler<T>>,
    // Reject
    Option<MetacallFutureHandler<T>>,
    // User data
    *mut T,
);

unsafe extern "C" fn resolver<T: 'static + Debug>(
    resolve_data: *mut c_void,
    upper_data: *mut c_void,
) -> *mut c_void {
    let (resolve, _, data) = *Box::from_raw(upper_data as *mut MetacallFutureFFIData<T>);
    let user_data = std::ptr::read_unaligned(data);
    let result = (resolve.unwrap())(
        parsers::raw_to_metacallobj_untyped_leak::<T>(resolve_data),
        user_data,
    );

    Box::into_raw(result) as *mut c_void
}
unsafe extern "C" fn rejecter<T: 'static + Debug>(
    reject_data: *mut c_void,
    upper_data: *mut c_void,
) -> *mut c_void {
    let (_, reject, data) = *Box::from_raw(upper_data as *mut MetacallFutureFFIData<T>);
    let user_data = std::ptr::read_unaligned(data);

    let result = (reject.unwrap())(
        parsers::raw_to_metacallobj_untyped_leak::<T>(reject_data),
        user_data,
    );

    Box::into_raw(result) as *mut c_void
}

impl<T: 'static + Debug> MetacallFuture<T> {
    fn create_null_data() -> *mut Option<T> {
        Box::into_raw(Box::new(None))
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
    ///
    ///
    /// **Usage example:**
    /// ```rust
    /// use metacall::{loaders, metacall_no_arg, switch, MetacallFuture, MetacallValue};
    /// let script = r#"
    /// function func_always_resolve(value, delay) {
    ///     return new Promise((resolve) => {
    ///         resolve(0);
    ///     });
    /// }
    ///
    /// module.exports = {
    ///     func_always_resolve
    /// }
    ///
    /// "#;
    /// let _metacall = switch::initialize().unwrap();
    /// loaders::from_memory("node", script).unwrap();
    ///
    /// fn resolve<T>(message: Box<dyn MetacallValue>, _: T) {
    ///     println!("[RESOLVE] {message?}")       
    /// }
    ///
    /// let future = metacall_no_arg::<MetacallFuture<()>>("func_always_resolve").unwrap();
    /// future
    ///     .then(resolve::<()>)
    ///     .await_fut();
    /// ```
    pub fn then(mut self, resolve: MetacallFutureHandler<T>) -> Self {
        self.resolve = Some(resolve);

        self
    }

    /// Adds a reject callback.
    ///
    /// **Usage example:**
    /// ```rust
    /// use metacall::{loaders, metacall_no_arg, switch, MetacallFuture, MetacallValue};
    /// let script = r#"
    /// function func_always_reject(value, delay) {
    ///     return new Promise((_, reject) => {
    ///         reject(0);
    ///     });
    /// }
    ///
    /// module.exports = {
    ///     func_always_reject
    /// }
    ///
    /// "#;
    /// let _metacall = switch::initialize().unwrap();
    /// loaders::from_memory("node", script).unwrap();
    ///
    /// fn reject<T>(message: Box<dyn MetacallValue>, _: T) {
    ///     println!("[REJECT] {message?}")       
    /// }
    ///
    /// let future = metacall_no_arg::<MetacallFuture<()>>("func_always_reject").unwrap();
    /// future
    ///     .catch(reject::<()>)
    ///     .await_fut();
    /// ```
    pub fn catch(mut self, reject: MetacallFutureHandler<T>) -> Self {
        self.reject = Some(reject);

        self
    }

    /// Adds data to use it inside the `resolver` and `reject`.
    /// ```rust
    /// // Nodejs Script
    /// let script = r#"
    /// function func(value, delay) {
    ///     return new Promise((resolve) => {
    ///         resolve(0);
    ///     });
    /// }
    ///
    /// module.exports = {
    ///     func
    /// }
    ///
    /// "#;
    ///
    /// let _metacall = switch::initialize().unwrap();
    /// loaders::from_memory("node", script).unwrap();
    ///
    /// fn resolve<T: PartialEq<String> + Debug>(_: Box<dyn MetacallValue>, data: T) {
    ///     assert_eq!(
    ///         data,
    ///         String::from("USER_DATA"),
    ///         "data should be passed without change"
    ///     );
    /// }
    ///
    /// let future = metacall_no_arg::<MetacallFuture<String>>("func").unwrap();
    /// future
    ///     .then(resolve::<String>)
    ///     .data(String::from("USER_DATA"))
    ///     .await_fut();
    ///}
    /// ```
    pub fn data(mut self, data: T) -> Self {
        dbg!(&data);
        unsafe { drop(Box::from_raw(self.data)) };
        dbg!(&self.data);
        self.data = Box::into_raw(Box::new(Some(data)));
        unsafe {
            dbg!(&*self.data);
        }
        self
    }

    /// Awaits the future.
    pub fn await_fut(self) {
        let resolve_is_some = self.resolve.is_some();
        let reject_is_some = self.reject.is_some();

        unsafe {
            dbg!(&*self.data);
        }
        unsafe {
            metacall_value_destroy(metacall_await_future(
                metacall_value_to_future(self.value),
                if resolve_is_some {
                    Some(resolver::<T>)
                } else {
                    None
                },
                if reject_is_some {
                    Some(rejecter::<T>)
                } else {
                    None
                },
                // TODO: Solve the memory leak that happens here
                // For reproducing the error, use the following commands:
                // cargo test --no-run
                // valgrind --trace-children=yes --leak-check=full --tool=memcheck --suppressions=../../../source/tests/memcheck/valgrind-node.supp ./target/debug/deps/metacall_test-248af33824f71bd1 &> output
                // ==20664== 60 (32 direct, 28 indirect) bytes in 1 blocks are definitely lost in loss record 11 of 35
                // ==20664==    at 0x4842839: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
                // ==20664==    by 0x17B549: alloc (alloc.rs:93)
                // ==20664==    by 0x17B549: alloc::alloc::Global::alloc_impl (alloc.rs:175)
                // ==20664==    by 0x17B342: allocate (alloc.rs:235)
                // ==20664==    by 0x17B342: alloc::alloc::exchange_malloc (alloc.rs:324)
                // ==20664==    by 0x1873D0: new<(core::option::Option<fn(alloc::boxed::Box<dyn metacall::types::metacall_value::MetacallValue, alloc::alloc::Global>, alloc::boxed::Box<dyn metacall::types::metacall_value::MetacallValue, alloc::alloc::Global>)>, core::option::Option<fn(alloc::boxed::Box<dyn metacall::types::metacall_value::MetacallValue, alloc::alloc::Global>, alloc::boxed::Box<dyn metacall::types::metacall_value::MetacallValue, alloc::alloc::Global>)>, *mut dyn metacall::types::metacall_value::MetacallValue)> (boxed.rs:217)
                // ==20664==    by 0x1873D0: metacall::types::metacall_future::MetacallFuture::await_fut (metacall_future.rs:182)
                // ==20664==    by 0x1296E6: metacall_test::test_future::{{closure}} (metacall_test.rs:202)
                // ==20664==    by 0x1286A2: metacall_test::generate_test_custom_validation (metacall_test.rs:42)
                // ==20664==    by 0x12625A: metacall_test::test_future (metacall_test.rs:193)
                // ==20664==    by 0x126954: metacall_test::metacall (metacall_test.rs:368)
                // ==20664==    by 0x129736: metacall_test::metacall::{{closure}} (metacall_test.rs:337)
                // ==20664==    by 0x1256B4: core::ops::function::FnOnce::call_once (function.rs:250)
                // ==20664==    by 0x166EBE: call_once<fn() -> core::result::Result<(), alloc::string::String>, ()> (function.rs:250)
                // ==20664==    by 0x166EBE: test::__rust_begin_short_backtrace (lib.rs:655)
                // ==20664==    by 0x13456B: {closure#1} (lib.rs:646)
                // ==20664==    by 0x13456B: core::ops::function::FnOnce::call_once{{vtable-shim}} (function.rs:250)
                self.into_raw(),
            ))
        };
    }

    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        unsafe {
            match std::ptr::read_unaligned(self.data) {
                Some(data) => {
                    Box::into_raw(Box::new((self.resolve, self.reject, data))) as *mut c_void
                }
                None => Box::into_raw(Box::new((self.resolve, self.reject, MetacallNull)))
                    as *mut c_void,
            }
        }
    }
}

impl<T> Drop for MetacallFuture<T> {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) };
        }
    }
}

#[cfg(test)]
mod tests {
    use std::fmt::Debug;

    use crate::{
        loaders, metacall, metacall_no_arg, switch, MetacallFuture, MetacallNull, MetacallValue,
    };
    #[test]
    fn test_metacall_future() {
        let script = r#"
function doubleValueAfterTime(value, delay) {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            if (typeof value === 'number') {
                resolve(value * 2); // Resolves if the value is a number
            } else {
                reject('Error: The provided value is not a number.'); // Rejects if the value is not a number
            }
        }, delay);
    });
}

module.exports = {
    doubleValueAfterTime
}
"#;

        let _metacall = switch::initialize().unwrap();
        loaders::from_memory("node", script).unwrap();
        for _ in 0..1000000 {
            fn resolve<T: PartialEq<i64> + Debug>(
                result: Box<dyn MetacallValue>,
                data: T,
            ) -> Box<dyn MetacallValue> {
                let result_f64 = result.clone().downcast::<f64>().unwrap();
                println!("data = {data:?}");
                assert_eq!(
                    result_f64, 2.0,
                    "the result should be double of the passed value"
                );
                assert_eq!(data, 100, "data should be passed without change");
                result
            }
            fn reject<T>(_: Box<dyn MetacallValue>, _: T) -> Box<dyn MetacallValue> {
                panic!("It shouldnt be rejected");
            }
            let future =
                metacall::<MetacallFuture<i64>>("doubleValueAfterTime", [1, 2000]).unwrap();
            future
                .then(resolve::<i64>)
                .catch(reject::<i64>)
                .data(100)
                .await_fut();
        }
    }

    #[test]
    fn test_metacall_future_data() {
        let _metacall = switch::initialize().unwrap();
        let script = r#"
function func(value, delay) {
    return new Promise((resolve) => {
        resolve(0);
    });
}

module.exports = {
    func
}
    "#;
        loaders::from_memory("node", script).unwrap();
        for _ in 0..1000000 {
            fn resolve<T: PartialEq<String> + Debug>(
                _: Box<dyn MetacallValue>,
                data: T,
            ) -> Box<dyn MetacallValue> {
                assert_eq!(
                    data,
                    String::from("USER_DATA"),
                    "data should be passed without change"
                );
                Box::new(MetacallNull())
            }

            let future = metacall_no_arg::<MetacallFuture<String>>("func").unwrap();
            future
                .then(resolve::<String>)
                .data(String::from("USER_DATA"))
                .await_fut();
        }
    }
}
