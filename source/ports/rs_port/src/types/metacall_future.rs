use self::cast::metacallobj_untyped_to_raw;

use super::{MetaCallNull, MetaCallValue};
use crate::{
    bindings::{
        metacall_await_future, metacall_value_create_null, metacall_value_destroy,
        metacall_value_to_future,
    },
    cast,
};
use std::{
    any::Any,
    ffi::c_void,
    fmt::{self, Debug, Formatter},
    ptr::null_mut,
};

/// Function pointer type used for resolving/rejecting MetaCall futures. The first argument is the result
/// and the second argument is the data that you may want to access when the function gets called.
/// Checkout [MetaCallFuture resolve](MetaCallFuture#method.then) or
/// [MetaCallFuture reject](MetaCallFuture#method.catch) for usage.
pub type MetaCallFutureHandler = fn(Box<dyn MetaCallValue>, Box<dyn Any>) -> Box<dyn MetaCallValue>;

/// Represents MetaCallFuture. Keep in mind that it's not supported to pass a future as an argument.
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
/// use metacall::{MetaCallValue, MetaCallFuture, metacall};
/// fn runner(x: i32) {
///
///     fn resolve(result: impl MetaCallValue, data: impl MetaCallValue) {
///         println!("Resolve:: result: {:#?}, data: {:#?}", result, data); //
///     }
///
///     fn reject(error: impl MetaCallValue, data: impl MetaCallValue) {
///         println!("Reject:: error: {:#?}, data: {:#?}", error, data);
///     }
///
///     let future = metacall::<MetaCallFuture>("doubleValueAfterTime", [1, 2000]).unwrap();
///     future.then(resolve).catch(reject).await_fut();
/// }
/// ```
#[repr(C)]
pub struct MetaCallFuture {
    data: *mut dyn Any,
    leak: bool,
    reject: Option<MetaCallFutureHandler>,
    resolve: Option<MetaCallFutureHandler>,
    value: *mut c_void,
}
unsafe impl Send for MetaCallFuture {}
unsafe impl Sync for MetaCallFuture {}
impl Clone for MetaCallFuture {
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
impl Debug for MetaCallFuture {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let boxed_data = unsafe { Box::from_raw(self.data) };
        let data = if boxed_data.is::<MetaCallNull>() {
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

        f.debug_struct("MetaCallFuture")
            .field("data", &data)
            .field("resolve", &resolve)
            .field("reject", &reject)
            .finish()
    }
}

type MetaCallFutureFFIData = (
    // Resolve
    Option<MetaCallFutureHandler>,
    // Reject
    Option<MetaCallFutureHandler>,
    // User data
    *mut dyn Any,
);

unsafe extern "C" fn resolver(resolve_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let (resolve, _, data) = *Box::from_raw(upper_data as *mut MetaCallFutureFFIData);
    let user_data = Box::from_raw(data);

    let result = (resolve.unwrap())(
        cast::raw_to_metacallobj_untyped_leak(resolve_data),
        user_data,
    );

    if let Some(ret) = metacallobj_untyped_to_raw(result) {
        return ret;
    }

    unsafe { metacall_value_create_null() }
}
unsafe extern "C" fn rejecter(reject_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let (_, reject, data) = *Box::from_raw(upper_data as *mut MetaCallFutureFFIData);
    let user_data = Box::from_raw(data);

    let result = (reject.unwrap())(
        cast::raw_to_metacallobj_untyped_leak(reject_data),
        user_data,
    );

    if let Some(ret) = metacallobj_untyped_to_raw(result) {
        return ret;
    }

    unsafe { metacall_value_create_null() }
}

impl MetaCallFuture {
    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        Self {
            data: null_mut::<()>(),
            leak: false,
            reject: None,
            resolve: None,
            value,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        Self {
            data: null_mut::<()>(),
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
    /// use metacall::{MetaCallValue, MetaCallFuture, metacall_no_args};
    /// fn calling() {
    ///     fn reject(result: impl MetaCallValue, _: impl MetaCallValue) {
    ///         println!("Resolve:: {:#?}", result); // Resolve:: "Resolve message"
    ///     }
    ///
    ///     let future = metacall_no_args::<MetaCallFuture>("func_always_resolve").unwrap();
    ///     future.then(resolve).catch(reject).await_fut();
    /// }
    /// ```
    pub fn then(mut self, resolve: MetaCallFutureHandler) -> Self {
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
    /// use metacall::{MetaCallValue, MetaCallFuture, metacall_no_args};
    /// fn calling() {
    ///     fn reject(error: impl MetaCallValue, _: impl MetaCallValue) {
    ///         println!("Reject:: error: {:#?}", error); // Reject:: error: "Error: Reject message"
    ///     }
    ///
    ///     let future = metacall_no_args::<MetaCallFuture>("func_always_rejects").unwrap();
    ///     future.then(resolve).catch(reject).await_fut();
    /// }
    /// ```
    pub fn catch(mut self, reject: MetaCallFutureHandler) -> Self {
        self.reject = Some(reject);

        self
    }

    /// Adds data to use it inside the `resolver` and `reject`.
    ///
    /// Example:
    /// ```rust
    /// use metacall::{MetaCallValue, MetaCallFuture, metacall};
    ///
    /// fn run() {
    ///   let x = 10;
    ///   fn resolve(result: impl MetaCallValue, data: impl MetaCallValue) {
    ///       println!("X = {data}");
    ///   }
    ///    
    ///   fn reject(result: impl MetaCallValue, data: impl MetaCallValue) {
    ///       println!("X = {data}");
    ///   }
    ///     
    ///   let future = metacall::<MetaCallFuture>("async_function", [1]).unwrap();
    ///   future.then(resolve).catch(reject),data(x).await_fut();
    /// }
    /// ```
    pub fn data<T: 'static>(mut self, data: T) -> Self {
        self.data = Box::into_raw(Box::new(data));

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
                    Some(resolver)
                } else {
                    None
                },
                if reject_is_some { Some(rejecter) } else { None },
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
                // ==20664==    by 0x1873D0: new<(core::option::Option<fn(alloc::boxed::Box<dyn metacall::types::metacall_value::MetaCallValue, alloc::alloc::Global>, alloc::boxed::Box<dyn metacall::types::metacall_value::MetaCallValue, alloc::alloc::Global>)>, core::option::Option<fn(alloc::boxed::Box<dyn metacall::types::metacall_value::MetaCallValue, alloc::alloc::Global>, alloc::boxed::Box<dyn metacall::types::metacall_value::MetaCallValue, alloc::alloc::Global>)>, *mut dyn metacall::types::metacall_value::MetaCallValue)> (boxed.rs:217)
                // ==20664==    by 0x1873D0: metacall::types::metacall_future::MetaCallFuture::await_fut (metacall_future.rs:182)
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

        panic!("Passing MetaCallFuture as an argument is not supported!");
    }
}

impl Drop for MetaCallFuture {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) };
        }
    }
}
