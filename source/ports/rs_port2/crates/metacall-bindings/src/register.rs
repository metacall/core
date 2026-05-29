use crate::value::MetacallValueID;

extern "C" {
    /// Registers a function by name `name` and arguments `va_args`.
    ///
    /// # Parameters
    /// - `name`: Name of the function (if it is `NULL`, the function is not registered into the host scope).
    /// - `invoke`: Pointer to the function invoke interface (argc, argv, data).
    /// - `func`: Will set the pointer to the function if the parameter is not null.
    /// - `return_type`: Type of return value.
    /// - `size`: Number of function arguments.
    /// - `va_args`: Variadic function parameter types.
    ///
    /// # Returns
    /// Pointer to the value containing the result of the call.
    pub fn metacall_register(
        name: *const ::std::os::raw::c_char,
        invoke: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: usize,
                arg2: *mut *mut ::std::os::raw::c_void,
                arg3: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        func: *mut *mut ::std::os::raw::c_void,
        return_type: MetacallValueID,
        size: usize,
        ...
    ) -> ::std::os::raw::c_int;

    /// Registers a function by name `name` and arguments `types`.
    ///
    /// # Parameters
    /// - `name`: Name of the function (if it is `NULL`, the function is not registered into the host scope).
    /// - `invoke`: Pointer to the function invoke interface (argc, argv, data).
    /// - `func`: Will set the pointer to the function if the parameter is not null.
    /// - `return_type`: Type of return value.
    /// - `size`: Number of function arguments.
    /// - `types`: List of parameter types.
    ///
    /// # Returns
    /// Pointer to the value containing the result of the call.
    pub fn metacall_registerv(
        name: *const ::std::os::raw::c_char,
        invoke: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: usize,
                arg2: *mut *mut ::std::os::raw::c_void,
                arg3: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        func: *mut *mut ::std::os::raw::c_void,
        return_type: MetacallValueID,
        size: usize,
        types: *mut MetacallValueID,
    ) -> ::std::os::raw::c_int;

    /// Registers a function by name `name` and arguments `types` within a specified loader.
    ///
    /// # Parameters
    /// - `loader`: Opaque pointer to the loader in which you want to register the function.
    /// - `handle`: Opaque pointer to the handle in which you want to register the function (if `NULL`, it will be defined in the global scope of the loader).
    /// - `name`: Name of the function (if `NULL`, the function is not registered into the host scope).
    /// - `invoke`: Pointer to the function invoke interface (argc, argv, data).
    /// - `return_type`: Type of return value.
    /// - `size`: Number of function arguments.
    /// - `types`: List of parameter types.
    ///
    /// # Returns
    /// Zero if the function was registered properly, or a non-zero value otherwise.
    pub fn metacall_register_loaderv(
        loader: *mut ::std::os::raw::c_void,
        handle: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        invoke: ::std::option::Option<
            unsafe extern "C" fn(
                arg1: usize,
                arg2: *mut *mut ::std::os::raw::c_void,
                arg3: *mut ::std::os::raw::c_void,
            ) -> *mut ::std::os::raw::c_void,
        >,
        return_type: MetacallValueID,
        size: usize,
        types: *mut MetacallValueID,
    ) -> ::std::os::raw::c_int;
}
