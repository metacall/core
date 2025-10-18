extern "C" {

    /// Obtains the loader instance by `tag`.
    ///
    /// # Parameters
    /// - `tag`: Tag in which the loader is identified, typically the extension of the script.
    ///
    /// # Returns
    /// Pointer to the loader by `tag`.
    pub fn metacall_loader(tag: *const ::std::os::raw::c_char) -> *mut ::std::os::raw::c_void;

    /// Loads a script from a file specified by the path.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script.
    /// - `paths`: Array of file paths.
    /// - `size`: Size of the array of paths.
    /// - `handle`: Optional pointer to a reference of the loaded handle.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_load_from_file(
        tag: *const ::std::os::raw::c_char,
        paths: *mut *const ::std::os::raw::c_char,
        size: usize,
        handle: *mut *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;

    /// Loads a script from memory.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script.
    /// - `buffer`: Memory block representing the script string.
    /// - `size`: Size of the memory block.
    /// - `handle`: Optional pointer to a reference of the loaded handle.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_load_from_memory(
        tag: *const ::std::os::raw::c_char,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        handle: *mut *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;

    /// Loads a package of scripts from a file specified by the path into the loader defined by the extension.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script.
    /// - `path`: Path of the package.
    /// - `handle`: Optional pointer to a reference of the loaded handle.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_load_from_package(
        tag: *const ::std::os::raw::c_char,
        path: *const ::std::os::raw::c_char,
        handle: *mut *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;

    /// Loads a list of scripts from a configuration specified by the path into the loader.
    ///
    /// The configuration format is:
    /// ```json
    /// {
    ///     "language_id": "<tag>",
    ///     "path": "<path>",
    ///     "scripts": ["<script0>", "<script1>", ..., "<scriptN>"]
    /// }
    /// ```
    ///
    /// # Parameters
    /// - `path`: Path of the configuration.
    /// - `handle`: Optional pointer to a reference of the loaded handle.
    /// - `allocator`: Pointer to the allocator that will allocate the configuration.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_load_from_configuration(
        path: *const ::std::os::raw::c_char,
        handle: *mut *mut ::std::os::raw::c_void,
        allocator: *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int;
}