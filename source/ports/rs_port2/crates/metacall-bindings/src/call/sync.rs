use crate::value::MetacallValueID;

extern "C" {
    /// Calls a function anonymously by value array `args`.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    /// - `args`: Array of pointers to data.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallv(
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by value array `args` with a specified size.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    /// - `args`: Array of pointers to data.
    /// - `size`: Number of elements of the call.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallv_s(
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by handle `handle` and value array `args`.
    ///
    /// # Parameters
    /// - `handle`: Handle where the function belongs.
    /// - `name`: Name of the function.
    /// - `args`: Array of pointers to data.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallhv(
        handle: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by handle `handle`, value array `args`, and a specified size.
    ///
    /// # Parameters
    /// - `handle`: Handle where the function belongs.
    /// - `name`: Name of the function.
    /// - `args`: Array of pointers to data.
    /// - `size`: Number of elements of the call.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallhv_s(
        handle: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by variable arguments `va_args`.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    /// - `va_args`: Variadic function parameters.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacall(name: *const ::std::os::raw::c_char, ...) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by type array `ids` and variable arguments `va_args`.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    /// - `ids`: Array of types referring to `va_args`.
    /// - `va_args`: Variadic function parameters.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallt(
        name: *const ::std::os::raw::c_char,
        ids: *const MetacallValueID,
        ...
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by type array `ids` with variable arguments `va_args` and size.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    /// - `ids`: Array of types referring to `va_args`.
    /// - `size`: Number of elements of the call.
    /// - `va_args`: Variadic function parameters.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallt_s(
        name: *const ::std::os::raw::c_char,
        ids: *const MetacallValueID,
        size: usize,
        ...
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by handle `handle`, type array `ids`, variable arguments `va_args`, and size.
    ///
    /// # Parameters
    /// - `handle`: Pointer to the handle returned by `metacall_load_from_*`.
    /// - `name`: Name of the function.
    /// - `ids`: Array of types referring to `va_args`.
    /// - `size`: Number of elements of the call.
    /// - `va_args`: Variadic function parameters.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallht_s(
        handle: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        ids: *const MetacallValueID,
        size: usize,
        ...
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by value array `args` and function `func`.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    /// - `args`: Array of pointers to data.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallfv(
        func: *mut ::std::os::raw::c_void,
        args: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by value array `args`, function `func`, and specified size.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    /// - `args`: Array of pointers to data.
    /// - `size`: Number of function arguments.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallfv_s(
        func: *mut ::std::os::raw::c_void,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by variable arguments `va_args` and function `func`.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallf(func: *mut ::std::os::raw::c_void, ...) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by function `func` and serial `buffer` of size `size`.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    /// - `buffer`: String representing an array to be deserialized into function arguments.
    /// - `size`: Size of the string `buffer`.
    /// - `allocator`: Pointer to the allocator that will allocate the value.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallfs(
        func: *mut ::std::os::raw::c_void,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        allocator: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by value map (`keys -> values`) and function `func`.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    /// - `keys`: Array of values representing argument keys.
    /// - `values`: Array of values representing argument values data.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallfmv(
        func: *mut ::std::os::raw::c_void,
        keys: *mut *mut ::std::os::raw::c_void,
        values: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a function anonymously by function `func` and serial `buffer` of size `size`.
    ///
    /// # Parameters
    /// - `func`: Reference to the function to be called.
    /// - `buffer`: String representing a map to be deserialized into function arguments.
    /// - `size`: Size of the string `buffer`.
    /// - `allocator`: Pointer to the allocator that will allocate the value.
    ///
    /// # Returns
    /// Pointer to a value containing the result of the call.
    pub fn metacallfms(
        func: *mut ::std::os::raw::c_void,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        allocator: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

}
