extern "C" {
    /// Executes an asynchronous call to the function and registers a callback to be executed when a future is resolved (this call blocks).
    ///
    /// # Parameters
    ///
    /// - `name`: The name of the function to be called asynchronously.
    /// - `args`: Array of pointers to the values to be passed to the function.
    /// - `resolve_callback`: Pointer to a function that will be executed when the task is completed.
    ///   - `arg1`: Value representing the result of the future resolution.
    ///   - `arg2`: A reference to `data` that will be used as a closure for the chain.
    ///   - Returns: Value containing the result of the operation, wrapped into a future.
    /// - `reject_callback`: Pointer to a function that will be executed in case of a task error (identical signature to `resolve_callback`).
    /// - `data`: Pointer to a context that will act as a closure for the chain.
    ///
    /// # Returns
    ///
    /// Pointer to the result value returned by either `resolve_callback` or `reject_callback`, wrapped in a future.
    pub fn metacall_await(
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Awaits a promise and registers a callback to be executed when a future is resolved.
    ///
    /// # Parameters
    ///
    /// - `f`: Pointer to the future.
    /// - `resolve_callback`: Function pointer executed upon task completion.
    ///   - `arg1`: Result of the future resolution.
    ///   - `arg2`: Reference to the `data` used as closure.
    /// - `reject_callback`: Function pointer executed in case of task error (same signature as `resolve_callback`).
    /// - `data`: Pointer to a context acting as closure for the chain.
    ///
    /// # Returns
    ///
    /// Pointer to the result value returned by `resolve_callback` or `reject_callback`, wrapped in a future.
    pub fn metacall_await_future(
        f: *mut ::std::os::raw::c_void,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Executes an asynchronous call with an argument size and registers a callback for the future resolution (blocking).
    ///
    /// # Parameters
    ///
    /// - `name`: The function name.
    /// - `args`: Array of argument pointers.
    /// - `size`: Number of elements in the `args` array.
    /// - `resolve_callback`: Function called on task completion (same as above).
    /// - `reject_callback`: Function called on task error (same as above).
    /// - `data`: Pointer to a context acting as a closure.
    ///
    /// # Returns
    ///
    /// Result value wrapped in a future returned by the callbacks.
    pub fn metacall_await_s(
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls an asynchronous function by passing a function pointer and argument array, awaiting the result.
    ///
    /// # Parameters
    ///
    /// - `func`: Pointer to the function.
    /// - `args`: Array of argument pointers.
    /// - `resolve_callback`: Function pointer executed when the task completes.
    /// - `reject_callback`: Function pointer executed when the task errors (identical to `resolve_callback`).
    /// - `data`: Closure context pointer.
    ///
    /// # Returns
    ///
    /// Pointer to the future result.
    pub fn metacallfv_await(
        func: *mut ::std::os::raw::c_void,
        args: *mut *mut ::std::os::raw::c_void,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls an asynchronous function with argument size and awaits the result.
    ///
    /// # Parameters
    ///
    /// - `func`: Function pointer.
    /// - `args`: Argument pointer array.
    /// - `size`: Number of elements in `args`.
    /// - `resolve_callback`: Pointer executed on task completion.
    /// - `reject_callback`: Pointer executed on task error.
    /// - `data`: Pointer to closure context.
    ///
    /// # Returns
    ///
    /// Pointer to the future result.
    pub fn metacallfv_await_s(
        func: *mut ::std::os::raw::c_void,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Executes an asynchronous function call using a map of keys and values, and awaits the result.
    ///
    /// # Parameters
    ///
    /// - `func`: Function pointer.
    /// - `keys`: Pointer array representing argument keys.
    /// - `values`: Pointer array representing argument values.
    /// - `size`: Size of the `keys` and `values` arrays.
    /// - `resolve_callback`: Called on task success.
    /// - `reject_callback`: Called on task error.
    /// - `data`: Context used as closure for the callbacks.
    ///
    /// # Returns
    ///
    /// Future-wrapped result.
    pub fn metacallfmv_await_s(
        func: *mut ::std::os::raw::c_void,
        keys: *mut *mut ::std::os::raw::c_void,
        values: *mut *mut ::std::os::raw::c_void,
        size: usize,
        resolve_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls an asynchronous function anonymously using the given function pointer and serialized argument buffer.
    ///
    /// # Parameters
    ///
    /// - `func`: Pointer to the function to be called.
    /// - `buffer`: String representing an array of serialized arguments.
    /// - `size`: Size of the `buffer`.
    /// - `allocator`: Pointer to the allocator used for allocating the result.
    /// - `resolve_callback`: Function pointer executed when the task completes successfully.
    ///   - `arg1`: Pointer to the result of the future resolution.
    ///   - `arg2`: Reference to `data`, which acts as a closure for the callback chain.
    ///   - Returns: Pointer to the result, wrapped in a future to be returned by the function.
    /// - `reject_callback`: Function pointer executed when the task fails (same signature as `resolve_callback`).
    /// - `data`: Pointer to a context that acts as a closure for the callback chain.
    ///
    /// # Returns
    ///
    /// Pointer to the result returned by `resolve_callback` or `reject_callback`, wrapped in a future.
    pub fn metacallfs_await(
        func: *mut ::std::os::raw::c_void,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        allocator: *mut ::std::os::raw::c_void,
        resolve_callback: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls an asynchronous function anonymously using the given function pointer and serialized map buffer.
    ///
    /// # Parameters
    ///
    /// - `func`: Pointer to the function to be called.
    /// - `buffer`: String representing a map of serialized arguments.
    /// - `size`: Size of the `buffer`.
    /// - `allocator`: Pointer to the allocator used for allocating the result.
    /// - `resolve_callback`: Function pointer executed when the task completes successfully.
    ///   - `arg1`: Pointer to the result of the future resolution.
    ///   - `arg2`: Reference to `data`, which acts as a closure for the callback chain.
    ///   - Returns: Pointer to the result, wrapped in a future to be returned by the function.
    /// - `reject_callback`: Function pointer executed when the task fails (same signature as `resolve_callback`).
    /// - `data`: Pointer to a context that acts as a closure for the callback chain.
    ///
    /// # Returns
    ///
    /// Pointer to the result returned by `resolve_callback` or `reject_callback`, wrapped in a future.
    pub fn metacallfms_await(
        func: *mut ::std::os::raw::c_void,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        allocator: *mut ::std::os::raw::c_void,
        resolve_callback: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        reject_callback: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: *mut ::std::os::raw::c_void,
                arg2: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        data: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;
}
