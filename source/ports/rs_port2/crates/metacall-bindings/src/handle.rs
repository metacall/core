extern "C" {
    /// Initializes an empty handler into a loader by `name`.
    ///
    /// # Parameters
    /// - `loader`: Pointer to the loader to which the handle belongs.
    /// - `name`: Name of the handle.
    /// - `handle_ptr`: On success, returns the pointer to the created handle, otherwise `NULL`.
    ///
    /// # Returns
    /// Zero on success, non-zero on error.
    pub fn metacall_handle_initialize(
        loader: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
        handle_ptr: *mut *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;

    /// Populates the objects of `handle_src` into `handle_dest`.
    ///
    /// # Parameters
    /// - `handle_dest`: Handle where the objects from `handle_src` will be stored.
    /// - `handle_src`: Handle from where the objects will be copied.
    ///
    /// # Returns
    /// Zero on success, non-zero on error.
    pub fn metacall_handle_populate(
        handle_dest: *mut ::std::os::raw::c_void,
        handle_src: *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;

    /// Retrieves a function by `name` from a specific `handle`.
    ///
    /// # Parameters
    /// - `handle`: Pointer to the handle returned by `metacall_load_from_*`.
    /// - `name`: Name of the function.
    ///
    /// # Returns
    /// Function reference, null if the function does not exist.
    pub fn metacall_handle_function(
        handle: *mut ::std::os::raw::c_void,
        name: *const ::std::os::raw::c_char,
    ) -> *mut ::std::os::raw::c_void;

    /// Retrieves the handle by `name`.
    ///
    /// # Parameters
    /// - `tag`: Extension of the script.
    /// - `name`: Name of the handle.
    ///
    /// # Returns
    /// Handle reference, or null if the function does not exist.
    pub fn metacall_handle(
        tag: *const ::std::os::raw::c_char,
        name: *const ::std::os::raw::c_char,
    ) -> *mut ::std::os::raw::c_void;

    /// Retrieves the name of a `handle`.
    ///
    /// # Parameters
    /// - `handle`: Pointer to the handle to be retrieved.
    ///
    /// # Returns
    /// String that references the handle.
    pub fn metacall_handle_id(handle: *mut ::std::os::raw::c_void)
        -> *const ::std::os::raw::c_char;

    /// Returns a value representing the handle as a map of functions (or values).
    ///
    /// # Parameters
    /// - `handle`: Reference to the handle to be described.
    ///
    /// # Returns
    /// A value of type map on success, or null otherwise.
    pub fn metacall_handle_export(
        handle: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

}
