pub mod cast;
pub mod create;
#[repr(C)]
pub enum MetacallValueID {
    MetacallBool = 0,
    MetacallChar = 1,
    MetacallShort = 2,
    MetacallInt = 3,
    MetacallLong = 4,
    MetacallFloat = 5,
    MetacallDouble = 6,
    MetacallString = 7,
    MetacallBuffer = 8,
    MetacallArray = 9,
    MetacallMap = 10,
    MetacallPtr = 11,
    MetacallFuture = 12,
    MetacallFunction = 13,
    MetacallNull = 14,
    MetacallClass = 15,
    MetacallObject = 16,
    MetacallException = 17,
    MetacallThrowable = 18,
    MetacallSize = 19,
    MetacallInvalid = 20,
}

extern "C" {
    /// Returns the size of the value in bytes.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The size of the value in bytes.
    pub fn metacall_value_size(v: *mut ::std::os::raw::c_void) -> usize;

    /// Returns the number of values that this value contains.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The number of values that the referenced value represents.
    pub fn metacall_value_count(v: *mut ::std::os::raw::c_void) -> usize;

    /// Provides the type identifier (ID) of the value.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The type ID assigned to the value.
    pub fn metacall_value_id(v: *mut ::std::os::raw::c_void) -> MetacallValueID;

    /// Provides the type identifier (ID) in a readable form (as a string).
    ///
    /// # Parameters
    /// * `id`: The value type identifier.
    ///
    /// # Returns
    /// A string related to the type ID.
    pub fn metacall_value_id_name(id: MetacallValueID) -> *const ::std::os::raw::c_char;

    /// Provides the type identifier (ID) in a readable form (as a string) for the value.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// A string related to the type ID assigned to the value.
    pub fn metacall_value_type_name(
        v: *mut ::std::os::raw::c_void,
    ) -> *const ::std::os::raw::c_char;

    /// Deep copies the given value, resetting its reference counter and ownership.
    ///
    /// # Parameters
    /// * `v`: Reference to the value to be copied.
    ///
    /// # Returns
    /// A copy of the value on success, or `null` otherwise.
    pub fn metacall_value_copy(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Transfers ownership from the source value to the destination value, including the finalizer.
    ///
    /// # Parameters
    /// * `src`: The source value that will lose ownership.
    /// * `dst`: The destination value that will receive ownership.
    pub fn metacall_value_move(src: *mut ::std::os::raw::c_void, dest: *mut ::std::os::raw::c_void);

    /// Converts the given value to a boolean.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a boolean.
    pub fn metacall_value_to_bool(v: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_uchar;

    /// Converts the given value to a character.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a character.
    pub fn metacall_value_to_char(v: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_char;

    /// Converts the given value to a short integer.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a short integer.
    pub fn metacall_value_to_short(v: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_short;

    /// Converts the given value to an integer.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to an integer.
    pub fn metacall_value_to_int(v: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_int;

    /// Converts the given value to a long integer.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a long integer.
    pub fn metacall_value_to_long(v: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_long;

    /// Converts the given value to a single-precision floating point.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a float.
    pub fn metacall_value_to_float(v: *mut ::std::os::raw::c_void) -> f32;

    /// Converts the given value to a double-precision floating point.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a double.
    pub fn metacall_value_to_double(v: *mut ::std::os::raw::c_void) -> f64;

    /// Converts the given value to a C string (null-terminated).
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a C string.
    pub fn metacall_value_to_string(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_char;

    /// Converts the given value to a memory block (buffer).
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a memory block.
    pub fn metacall_value_to_buffer(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to an array of values.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to an array of values.
    pub fn metacall_value_to_array(
        v: *mut ::std::os::raw::c_void,
    ) -> *mut *mut ::std::os::raw::c_void;

    /// Converts the given value to a map (array of tuples).
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a map (array of tuples, which are arrays of values).
    pub fn metacall_value_to_map(
        v: *mut ::std::os::raw::c_void,
    ) -> *mut *mut ::std::os::raw::c_void;

    /// Converts the given value to a pointer.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a pointer.
    pub fn metacall_value_to_ptr(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to a future.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a future.
    pub fn metacall_value_to_future(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to a function.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a function.
    pub fn metacall_value_to_function(
        v: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to null.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to null.
    pub fn metacall_value_to_null(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to a class.
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to a class.
    pub fn metacall_value_to_class(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to an object (instance of a class).
    ///
    /// # Parameters
    /// * `v`: Reference to the value.
    ///
    /// # Returns
    /// The value converted to an object.
    pub fn metacall_value_to_object(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to an exception.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be converted.
    ///
    /// # Returns
    /// A pointer to the value converted to an exception.
    pub fn metacall_value_to_exception(
        v: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Converts the given value to a throwable.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be converted.
    ///
    /// # Returns
    /// A pointer to the value converted to a throwable.
    pub fn metacall_value_to_throwable(
        v: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a boolean value to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `b`: The boolean value to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned boolean.
    pub fn metacall_value_from_bool(
        v: *mut ::std::os::raw::c_void,
        b: ::std::os::raw::c_uchar,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a character to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `c`: The character to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned character.
    pub fn metacall_value_from_char(
        v: *mut ::std::os::raw::c_void,
        c: ::std::os::raw::c_char,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a short integer to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `s`: The short integer to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned short integer.
    pub fn metacall_value_from_short(
        v: *mut ::std::os::raw::c_void,
        s: ::std::os::raw::c_short,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns an integer to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `i`: The integer to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned integer.
    pub fn metacall_value_from_int(
        v: *mut ::std::os::raw::c_void,
        i: ::std::os::raw::c_int,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a long integer to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `l`: The long integer to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned long integer.
    pub fn metacall_value_from_long(
        v: *mut ::std::os::raw::c_void,
        l: ::std::os::raw::c_long,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a single-precision floating-point number to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `f`: The float to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned float.
    pub fn metacall_value_from_float(
        v: *mut ::std::os::raw::c_void,
        f: f32,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a double-precision floating-point number to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `d`: The double to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned double.
    pub fn metacall_value_from_double(
        v: *mut ::std::os::raw::c_void,
        d: f64,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a string to the specified value, truncating if necessary.
    ///
    /// This function does not add a null terminator if the string is truncated.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `str_`: A pointer to the null-terminated string to assign.
    /// - `length`: The length of the string.
    ///
    /// # Returns
    /// A pointer to the value with the assigned string.
    pub fn metacall_value_from_string(
        v: *mut ::std::os::raw::c_void,
        str_: *const ::std::os::raw::c_char,
        length: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns an array to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `buffer`: A pointer to the constant array to assign.
    /// - `size`: The number of elements in the array.
    ///
    /// # Returns
    /// A pointer to the value with the assigned array.
    pub fn metacall_value_from_buffer(
        v: *mut ::std::os::raw::c_void,
        buffer: *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns an array of values to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `values`: A pointer to the constant array of values to assign.
    /// - `size`: The number of values in the array.
    ///
    /// # Returns
    /// A pointer to the value with the assigned array of values.
    pub fn metacall_value_from_array(
        v: *mut ::std::os::raw::c_void,
        values: *mut *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns an array of tuples to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `tuples`: A pointer to the constant array of tuples to assign.
    /// - `size`: The number of tuples in the array.
    ///
    /// # Returns
    /// A pointer to the value with the assigned array of tuples.
    pub fn metacall_value_from_map(
        v: *mut ::std::os::raw::c_void,
        tuples: *mut *const ::std::os::raw::c_void,
        size: usize,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a pointer to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `ptr`: A pointer to be assigned to the value.
    ///
    /// # Returns
    /// A pointer to the value with the assigned pointer.
    pub fn metacall_value_from_ptr(
        v: *mut ::std::os::raw::c_void,
        ptr: *const ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a future to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `f`: A pointer to the future to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned future.
    pub fn metacall_value_from_future(
        v: *mut ::std::os::raw::c_void,
        f: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a function to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    /// - `f`: A pointer to the function to assign.
    ///
    /// # Returns
    /// A pointer to the value with the assigned function.
    pub fn metacall_value_from_function(
        v: *mut ::std::os::raw::c_void,
        f: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns a null value to the specified value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be assigned.
    ///
    /// # Returns
    /// A pointer to the value with the assigned null.
    pub fn metacall_value_from_null(v: *mut ::std::os::raw::c_void) -> *mut ::std::os::raw::c_void;

    /// Assigns the specified class to the given value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to which the class will be assigned.
    /// - `c`: A pointer to the class to be assigned to the value.
    ///
    /// # Returns
    /// A pointer to the value with the assigned class.
    pub fn metacall_value_from_class(
        v: *mut ::std::os::raw::c_void,
        c: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns the specified object to the given value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to which the object will be assigned.
    /// - `o`: A pointer to the object to be assigned to the value.
    ///
    /// # Returns
    /// A pointer to the value with the assigned object.
    pub fn metacall_value_from_object(
        v: *mut ::std::os::raw::c_void,
        o: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns the specified exception to the given value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to which the exception will be assigned.
    /// - `ex`: A pointer to the exception to be assigned to the value.
    ///
    /// # Returns
    /// A pointer to the value with the assigned exception.
    pub fn metacall_value_from_exception(
        v: *mut ::std::os::raw::c_void,
        ex: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Assigns the specified throwable to the given value.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to which the throwable will be assigned.
    /// - `th`: A pointer to the throwable to be assigned to the value.
    ///
    /// # Returns
    /// A pointer to the value with the assigned throwable.
    pub fn metacall_value_from_throwable(
        v: *mut ::std::os::raw::c_void,
        th: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Destroys the specified value from the scope stack.
    ///
    /// # Parameters
    /// - `v`: A pointer to the value to be destroyed.
    pub fn metacall_value_destroy(v: *mut ::std::os::raw::c_void);
}
