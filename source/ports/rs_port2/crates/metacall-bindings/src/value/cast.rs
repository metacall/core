extern "C" {
    /// Casts a value to a new type specified by `id`.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be cast.
    /// - `id`: The new type ID of the value to be cast.
    ///
    /// # Returns
    /// A pointer to the casted value, or a reference to `v` if the casting is between equivalent types.
    pub fn metacall_value_cast(
        v: *mut ::std::os::raw::c_void,
        id: super::MetacallValueID,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a boolean.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a boolean.
    pub fn metacall_value_cast_bool(v: *mut *mut ::std::os::raw::c_void)
        -> ::std::os::raw::c_uchar;

    /// Implicitly converts the value `v` to a character.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a character.
    pub fn metacall_value_cast_char(v: *mut *mut ::std::os::raw::c_void) -> ::std::os::raw::c_char;

    /// Implicitly converts the value `v` to a short integer.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a short integer.
    pub fn metacall_value_cast_short(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_short;

    /// Implicitly converts the value `v` to an integer.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to an integer.
    pub fn metacall_value_cast_int(v: *mut *mut ::std::os::raw::c_void) -> ::std::os::raw::c_int;

    /// Implicitly converts the value `v` to a long integer.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a long integer.
    pub fn metacall_value_cast_long(v: *mut *mut ::std::os::raw::c_void) -> ::std::os::raw::c_long;

    /// Implicitly converts the value `v` to a single precision floating point.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a single precision floating point.
    pub fn metacall_value_cast_float(v: *mut *mut ::std::os::raw::c_void) -> f32;

    /// Implicitly converts the value `v` to a double precision floating point.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// The value converted to a double precision floating point.
    pub fn metacall_value_cast_double(v: *mut *mut ::std::os::raw::c_void) -> f64;

    /// Implicitly converts the value `v` to a C string (null-terminated).
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted C string.
    pub fn metacall_value_cast_string(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_char;

    /// Implicitly converts the value `v` to a buffer.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted buffer.
    pub fn metacall_value_cast_buffer(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to an array of values.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted array.
    pub fn metacall_value_cast_array(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a map.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted map.
    pub fn metacall_value_cast_map(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a pointer.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted value.
    pub fn metacall_value_cast_ptr(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a future.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted future.
    pub fn metacall_value_cast_future(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a function.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted function.
    pub fn metacall_value_cast_function(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to null.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted null value.
    pub fn metacall_value_cast_null(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a class.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted class.
    pub fn metacall_value_cast_class(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to an object.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted object.
    pub fn metacall_value_cast_object(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to an exception.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted exception.
    pub fn metacall_value_cast_exception(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Implicitly converts the value `v` to a throwable.
    ///
    /// # Parameters
    /// - `v`: A pointer to a pointer of the value to be converted.
    ///
    /// # Returns
    /// A pointer to the converted throwable.
    pub fn metacall_value_cast_throwable(
        v: *mut *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;
}
