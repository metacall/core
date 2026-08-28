extern "C" {
    /// Creates a value from a boolean.
    ///
    /// # Parameters
    /// - `b`: Boolean value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_bool(b: ::std::os::raw::c_uchar) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a character.
    ///
    /// # Parameters
    /// - `c`: Character value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_char(c: ::std::os::raw::c_char) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a short integer.
    ///
    /// # Parameters
    /// - `s`: Short integer that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_short(s: ::std::os::raw::c_short) -> *mut ::std::os::raw::c_void;

    /// Creates a value from an integer.
    ///
    /// # Parameters
    /// - `i`: Integer value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_int(i: ::std::os::raw::c_int) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a long integer.
    ///
    /// # Parameters
    /// - `l`: Long integer value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_long(l: ::std::os::raw::c_long) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a single-precision floating point number.
    ///
    /// # Parameters
    /// - `f`: Float value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_float(f: f32) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a double-precision floating point number.
    ///
    /// # Parameters
    /// - `d`: Double value that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_double(d: f64) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a constant C string.
    ///
    /// # Parameters
    /// - `str_`: Constant string (must be null-terminated) that will be copied into the value.
    /// - `length`: Length of the constant string.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_string(
        str_: *const ::std::os::raw::c_char,
        length: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value buffer from a memory array.
    ///
    /// # Parameters
    /// - `buffer`: Constant memory block that will be copied into the value.
    /// - `size`: Size in bytes of the data contained in the array.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_buffer(
        buffer: *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value array from an array of values.
    ///
    /// # Parameters
    /// - `values`: Constant array of values that will be copied into the value list.
    /// - `size`: Number of elements contained in the array.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_array(
        values: *mut *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value map from an array of key-value tuples.
    ///
    /// # Parameters
    /// - `tuples`: Constant array of tuples that will be copied into the value map.
    /// - `size`: Number of elements contained in the map.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_map(
        tuples: *mut *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a pointer.
    ///
    /// # Parameters
    /// - `ptr`: Pointer to constant data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_ptr(
        ptr: *const ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a future object.
    ///
    /// # Parameters
    /// - `f`: Pointer to constant data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_future(
        f: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a function.
    ///
    /// # Parameters
    /// - `f`: Pointer to the function data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_function(
        f: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a function, binding a closure to it.
    ///
    /// # Parameters
    /// - `f`: Pointer to the function data that will be copied into the value.
    /// - `c`: Pointer to the closure to bind to the function.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_function_closure(
        f: *mut ::std::os::raw::c_void,
        c: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value of type null.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_null() -> *mut ::std::os::raw::c_void;

    /// Creates a value from a class object.
    ///
    /// # Parameters
    /// - `c`: Pointer to the class data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_class(
        c: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from an object.
    ///
    /// # Parameters
    /// - `o`: Pointer to the object data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_object(
        o: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from an exception.
    ///
    /// # Parameters
    /// - `ex`: Pointer to the exception data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_exception(
        ex: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a value from a throwable object.
    ///
    /// # Parameters
    /// - `th`: Pointer to the throwable data that will be copied into the value.
    ///
    /// # Returns
    /// A pointer to the created value on success, or null otherwise.
    pub fn metacall_value_create_throwable(
        th: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;
}
