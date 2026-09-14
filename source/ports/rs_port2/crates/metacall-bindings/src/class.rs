use crate::value::MetacallValueID;

extern "C" {
    /// Retrieves a class by its name.
    ///
    /// # Parameters
    ///
    /// - `name`: The name of the class.
    ///
    /// # Returns
    ///
    /// Pointer to the class reference, or `null` if the class does not exist.
    pub fn metacall_class(name: *const ::std::os::raw::c_char) -> *mut ::std::os::raw::c_void;

    /// Calls a class method anonymously using an array of arguments.
    ///
    /// This function assumes no method overloading and performs type conversion on the values.
    ///
    /// # Parameters
    ///
    /// - `cls`: Pointer to the class.
    /// - `name`: Name of the method to call.
    /// - `args`: Array of pointers to the method's arguments.
    /// - `size`: Number of elements in the `args` array.
    ///
    /// # Returns
    ///
    /// Pointer to the result of the method call.
    pub fn metacallv_class(
        cls: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls a class method anonymously with an expected return type, aiding in method overloading resolution.
    ///
    /// # Parameters
    ///
    /// - `cls`: Pointer to the class.
    /// - `name`: Name of the method to call.
    /// - `ret`: Expected return value type.
    /// - `args`: Array of pointers to the method's arguments.
    /// - `size`: Number of elements in the `args` array.
    ///
    /// # Returns
    ///
    /// Pointer to the result of the method call.
    pub fn metacallt_class(
        cls: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        ret: MetacallValueID,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Creates a new object instance from a class using the given constructor arguments.
    ///
    /// # Parameters
    ///
    /// - `cls`: Pointer to the class.
    /// - `name`: Name of the new object.
    /// - `args`: Array of pointers to the constructor parameters.
    /// - `size`: Number of elements in the `args` array.
    ///
    /// # Returns
    ///
    /// Pointer to the newly created object instance.
    pub fn metacall_class_new(
        cls: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Retrieves a class attribute by its key name.
    ///
    /// # Parameters
    ///
    /// - `cls`: Pointer to the class.
    /// - `key`: Name of the attribute to retrieve.
    ///
    /// # Returns
    ///
    /// Pointer to the class attribute value, or `NULL` if an error occurred.
    pub fn metacall_class_static_get(
        cls: *mut ::std::os::raw::c_void,
        key: *const ::std::os::raw::c_char,
    ) -> *mut ::std::os::raw::c_void;

    /// Sets a class attribute by its key name.
    ///
    /// # Parameters
    ///
    /// - `cls`: Pointer to the class.
    /// - `key`: Name of the attribute to set.
    /// - `value`: Value to set.
    ///
    /// # Returns
    ///
    /// A non-zero integer if an error occurred.
    pub fn metacall_class_static_set(
        cls: *mut ::std::os::raw::c_void,
        key: *const ::std::os::raw::c_char,
        v: *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;
}
