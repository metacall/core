use crate::value::MetacallValueID;

extern "C" {

    /// Retrieves a function by `name`.
    ///
    /// # Parameters
    /// - `name`: Name of the function.
    ///
    /// # Returns
    /// Function reference, null if the function does not exist.
    pub fn metacall_function(name: *const ::std::os::raw::c_char) -> *mut ::std::os::raw::c_void;

    /// Retrieves the function parameter type ID.
    ///
    /// # Parameters
    /// - `func`: Pointer to the function obtained from `metacall_function`.
    /// - `parameter`: Index of the parameter to be retrieved.
    /// - `id`: Parameter type ID that will be returned.
    ///
    /// # Returns
    /// Zero if the parameter index exists and function is valid, non-zero otherwise.
    pub fn metacall_function_parameter_type(
        func: *mut ::std::os::raw::c_void,
        parameter: usize,
        id: *mut MetacallValueID,
    ) -> ::std::os::raw::c_int;

    /// Retrieves the function return type ID.
    ///
    /// # Parameters
    /// - `func`: Pointer to the function obtained from `metacall_function`.
    /// - `id`: Return type ID of the function.
    ///
    /// # Returns
    /// Zero if the function is valid, non-zero otherwise.
    pub fn metacall_function_return_type(
        func: *mut ::std::os::raw::c_void,
        id: *mut MetacallValueID,
    ) -> ::std::os::raw::c_int;

    /// Retrieves the minimum number of arguments accepted by the function.
    ///
    /// # Parameters
    /// - `func`: Function reference.
    ///
    /// # Returns
    /// Number of arguments.
    pub fn metacall_function_size(func: *mut ::std::os::raw::c_void) -> usize;

    /// Checks if the function is asynchronous or synchronous.
    ///
    /// # Parameters
    /// - `func`: Function reference.
    ///
    /// # Returns
    /// - `true` if the function is asynchronous.
    /// - `false` if the function is synchronous.
    pub fn metacall_function_async(func: *mut ::std::os::raw::c_void) -> bool;
}
