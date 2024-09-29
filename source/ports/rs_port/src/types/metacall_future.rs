use super::{MetacallNull, MetacallValue};
use crate::{
    bindings::{metacall_await_future, metacall_value_destroy, metacall_value_to_future},
    helpers, parsers,
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
pub type MetacallFutureHandler<T> = fn(Box<dyn MetacallValue>, T);

/// Represents MetacallFuture. Keep in mind that it's not supported to pass a future as an argument.
///
/// ## **Usage example:**
///
/// **Javascript Code:**
/// ```javascript
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
/// ```
///
/// **Calling Example:**
/// ```rust
/// use metacall::{MetacallValue, MetacallFuture, metacall};
/// fn runner(x: i32) {
///
///     fn resolve(result: impl MetacallValue, data: impl MetacallValue) {
///         println!("Resolve:: result: {:#?}, data: {:#?}", result, data); //
///     }
///
///     fn reject(error: impl MetacallValue, data: impl MetacallValue) {
///         println!("Reject:: error: {:#?}, data: {:#?}", error, data);
///     }
///
///     let future = metacall::<MetacallFuture>("doubleValueAfterTime", [1, 2000]).unwrap();
///     future.then(resolve).catch(reject).await_fut();
/// }
/// ```
#[repr(C)]
pub struct MetacallFuture<T> {
    data: *mut dyn MetacallValue,
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
impl<T> Debug for MetacallFuture<T> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let boxed_data = unsafe { Box::from_raw(self.data) };
        let data = if boxed_data.is::<MetacallNull>() {
            None
        } else {
            Some(format!("{:#?}", boxed_data))
        };
        Box::leak(boxed_data);

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

unsafe extern "C" fn resolver<T: 'static>(
    resolve_data: *mut c_void,
    upper_data: *mut c_void,
) -> *mut c_void {
    let (resolve, _, data) = *Box::from_raw(upper_data as *mut MetacallFutureFFIData<T>);
    let user_data = Box::from_raw(data);

    (resolve.unwrap())(
        parsers::raw_to_metacallobj_untyped_leak::<T>(resolve_data),
        *user_data,
    );

    ptr::null_mut()
}
unsafe extern "C" fn rejecter<T: 'static>(
    reject_data: *mut c_void,
    upper_data: *mut c_void,
) -> *mut c_void {
    let (_, reject, data) = *Box::from_raw(upper_data as *mut MetacallFutureFFIData<T>);
    let user_data = Box::from_raw(data);

    (reject.unwrap())(
        parsers::raw_to_metacallobj_untyped_leak::<T>(reject_data),
        *user_data,
    );

    ptr::null_mut()
}

impl<T: 'static> MetacallFuture<T> {
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
    ///
    /// ## **Usage example:**
    ///
    ///
    /// ```javascript
    /// // Javascript script
    ///
    /// function func_always_rejects(value, delay) {
    ///     return new Promise((resolve) => {
    ///         resolve('Resolve message.');
    ///     });
    /// }
    /// ```
    /// **Calling Example:**
    ///
    /// ```rust
    /// use metacall::{MetacallValue, MetacallFuture, metacall_no_args};
    /// fn calling() {
    ///     fn reject(result: impl MetacallValue, _: impl MetacallValue) {
    ///         println!("Resolve:: {:#?}", result); // Resolve:: "Resolve message"
    ///     }
    ///
    ///     let future = metacall_no_args::<MetacallFuture<_>>("func_always_resolve").unwrap();
    ///     future.then(resolve).catch(reject).await_fut();
    /// }
    /// ```
    pub fn then(mut self, resolve: MetacallFutureHandler<T>) -> Self {
        self.resolve = Some(resolve);

        self
    }

    /// Adds a reject callback.
    ///
    /// ## **Usage example:**
    ///
    /// ```javascript
    /// // Javascript script
    /// function func_always_rejects(value, delay) {
    ///     return new Promise((_, reject) => {
    ///         reject('Error: Reject message.');
    ///     });
    /// }
    /// ```
    /// **Calling Example:**
    /// ```rust
    /// use metacall::{MetacallValue, MetacallFuture, metacall_no_args};
    /// fn calling() {
    ///     fn reject(error: impl MetacallValue, _: impl MetacallValue) {
    ///         println!("Reject:: error: {:#?}", error); // Reject:: error: "Error: Reject message"
    ///     }
    ///
    ///     let future = metacall_no_args::<MetacallFuture>("func_always_rejects").unwrap();
    ///     future.then(resolve).catch(reject).await_fut();
    /// }
    /// ```
    pub fn catch(mut self, reject: MetacallFutureHandler<T>) -> Self {
        self.reject = Some(reject);

        self
    }

    /// Adds data to use it inside the `resolver` and `reject`.
    ///
    /// Example:
    /// ```rust
    /// use metacall::{MetacallValue, MetacallFuture, metacall};
    ///
    /// fn run() {
    ///   let x = 10;
    ///   fn resolve(result: impl MetacallValue, data: impl MetacallValue) {
    ///       println!("X = {data}");
    ///   }
    ///    
    ///   fn reject(result: impl MetacallValue, data: impl MetacallValue) {
    ///       println!("X = {data}");
    ///   }
    ///     
    ///   let future = metacall::<MetacallFuture>("async_function", [1]).unwrap();
    ///   future.then(resolve).catch(reject),data(x).await_fut();
    /// }
    /// ```
    pub fn data(mut self, data: impl MetacallValue) -> Self {
        unsafe { drop(Box::from_raw(self.data)) };

        self.data = Box::into_raw(Box::new(data) as Box<dyn MetacallValue>);

        self
    }

    /// Awaits the future.
    pub fn await_fut(self) {
        let resolve_is_some = self.resolve.is_some();
        let reject_is_some = self.reject.is_some();

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
                Box::into_raw(Box::new((self.resolve, self.reject, self.data))) as *mut c_void,
            ))
        };
    }

    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        // TODO:
        // It's not implemented in any loader as the time of writing this block of code.
        // Feel free to implement as any loader adopted accepting Future as an argument.

        panic!("Passing MetacallFuture as an argument is not supported!");
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

    use crate::{loaders, metacall, metacall_no_arg, switch, MetacallFuture, MetacallValue};
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

        fn resolve<T: PartialEq<i16> + Debug>(result: Box<dyn MetacallValue>, data: T) {
            let result = result.downcast::<f64>().unwrap();

            assert_eq!(
                result, 2.0,
                "the result should be double of the passed value"
            );
            assert_eq!(data, 100, "data should be passed without change");
        }
        fn reject<T>(_: Box<dyn MetacallValue>, _: T) {
            panic!("It shouldnt be rejected");
        }
        let future = metacall::<MetacallFuture<i16>>("doubleValueAfterTime", [1, 2000]).unwrap();
        future
            .then(resolve::<i16>)
            .catch(reject::<i16>)
            .data(100)
            .await_fut();
    }

    #[test]
    fn test_metacall_future_data() {
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
        let _metacall = switch::initialize().unwrap();
        loaders::from_memory("node", script).unwrap();

        fn resolve<T: PartialEq<String> + Debug>(_: Box<dyn MetacallValue>, data: T) {
            assert_eq!(
                data,
                String::from("USER_DATA"),
                "data should be passed without change"
            );
        }

        let future = metacall_no_arg::<MetacallFuture<String>>("func").unwrap();
        future
            .then(resolve::<String>)
            .data(String::from("USER_DATA"))
            .await_fut();
    }
}
