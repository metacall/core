use crate::value::MetacallValueID;

extern "C" {
    /// Calls an object method anonymously using an array of arguments.
    ///
    /// This function assumes no method overloading and performs type conversion on the values.
    ///
    /// # Parameters
    ///
    /// - `obj`: Pointer to the object.
    /// - `name`: Name of the method to call.
    /// - `args`: Array of pointers to the method's arguments.
    /// - `size`: Number of elements in the `args` array.
    ///
    /// # Returns
    ///
    /// Pointer to the result of the method call.
    pub fn metacallv_object(
        obj: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Calls an object method anonymously with an expected return type, aiding in method overloading resolution.
    ///
    /// # Parameters
    ///
    /// - `obj`: Pointer to the object.
    /// - `name`: Name of the method to call.
    /// - `ret`: Expected return value type.
    /// - `args`: Array of pointers to the method's arguments.
    /// - `size`: Number of elements in the `args` array.
    ///
    /// # Returns
    ///
    /// Pointer to the result of the method call.
    pub fn metacallt_object(
        obj: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        ret: MetacallValueID,
        args: *mut *mut ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Retrieves an attribute from an object by its key name.
    ///
    /// # Parameters
    ///
    /// - `obj`: Pointer to the object.
    /// - `key`: Name of the attribute to retrieve.
    ///
    /// # Returns
    ///
    /// Pointer to the object attribute value, or `NULL` if an error occurred.
    pub fn metacall_object_get(
        obj: *mut ::std::os::raw::c_void,
        key: *const ::std::os::raw::c_char,
    ) -> *mut ::std::os::raw::c_void;

    /// Sets an attribute on an object by its key name.
    ///
    /// # Parameters
    ///
    /// - `obj`: Pointer to the object.
    /// - `key`: Name of the attribute to set.
    /// - `value`: Value to set.
    ///
    /// # Returns
    ///
    /// A non-zero integer if an error occurred.
    pub fn metacall_object_set(
        obj: *mut ::std::os::raw::c_void,
        key: *const ::std::os::raw::c_char,
        v: *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;
}
