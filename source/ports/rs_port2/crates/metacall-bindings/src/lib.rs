pub mod call;
pub mod class;
pub mod func;
pub mod handle;
pub mod loader;
pub mod value;
pub mod object;
pub mod register;
pub mod version;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct MetacallInitializeConfigurationType {
    pub tag: *mut ::std::os::raw::c_char,
    pub options: *mut ::std::os::raw::c_void,
}

extern "C" {
    /// Returns the default serializer used by MetaCall.
    ///
    /// # Returns
    /// A pointer to a string containing the name of the serializer to be used with serialization methods.
    pub fn metacall_serial() -> *const ::std::os::raw::c_char;

    /// Disables MetaCall logs. Must be called before `metacall_initialize`.
    ///
    /// When initializing MetaCall, a default log is set to stdout if none is defined.
    /// To benchmark or disable this default log, call this function before `metacall_initialize`.
    pub fn metacall_log_null();

    /// Sets flags in the MetaCall library.
    ///
    /// # Parameters
    /// - `flags`: A combination of flags referring to definitions `METACALL_FLAGS_*`.
    pub fn metacall_flags(flags: ::std::os::raw::c_int);

    /// Initializes the MetaCall library.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_initialize() -> ::std::os::raw::c_int;

    /// Initializes the MetaCall library with configuration arguments.
    ///
    /// # Parameters
    /// - `initialize_config`: Pointer to a configuration structure to be loaded in memory with data to be injected.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_initialize_ex(
        initialize_config: *mut MetacallInitializeConfigurationType,
    ) -> ::std::os::raw::c_int;

    /// Initializes MetaCall application arguments.
    ///
    /// # Parameters
    /// - `argc`: Number of additional parameters to be passed to the runtime.
    /// - `argv`: Additional parameters to be passed to the runtime when initializing.
    pub fn metacall_initialize_args(
        argc: ::std::os::raw::c_int,
        argv: *mut *mut ::std::os::raw::c_char,
    );

    /// Gets the number of arguments in which MetaCall was initialized.
    ///
    /// # Returns
    /// An integer equal to or greater than zero.
    pub fn metacall_argc() -> ::std::os::raw::c_int;

    /// Gets the arguments in which MetaCall was initialized.
    ///
    /// # Returns
    /// A pointer to an array of strings with the additional arguments.
    pub fn metacall_argv() -> *mut *mut ::std::os::raw::c_char;

    /// Checks if a script context is loaded by the specified tag.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script (if NULL, returns the status of the whole MetaCall instance).
    ///
    /// # Returns
    /// - `0` if context is initialized; different from `0` otherwise.
    pub fn metacall_is_initialized(tag: *const ::std::os::raw::c_char) -> ::std::os::raw::c_int;

    /// Gets the number of function call arguments supported by MetaCall.
    ///
    /// # Returns
    /// The number of supported arguments.
    pub fn metacall_args_size() -> usize;

    /// Sets an execution path for the extension script specified by the tag.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script.
    /// - `path`: The path to be loaded.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_execution_path(
        tag: *const ::std::os::raw::c_char,
        path: *const ::std::os::raw::c_char,
    ) -> ::std::os::raw::c_int;

    /// Sets an execution path for the extension script specified by the tag with length.
    ///
    /// # Parameters
    /// - `tag`: The extension of the script.
    /// - `tag_length`: Length of the extension of the tag.
    /// - `path`: The path to be loaded.
    /// - `path_length`: Length of the path.
    ///
    /// # Returns
    /// - `0` if success; different from `0` otherwise.
    pub fn metacall_execution_path_s(
        tag: *const ::std::os::raw::c_char,
        tag_length: usize,
        path: *const ::std::os::raw::c_char,
        path_length: usize,
    ) -> ::std::os::raw::c_int;

    /// Clears a handle from memory and unloads related resources.
    ///
    /// # Parameters
    ///
    /// - `handle`: Pointer to the handle to be unloaded.
    ///
    /// # Returns
    ///
    /// Returns zero if successful, and a non-zero value otherwise.
    pub fn metacall_clear(handle: *mut ::std::os::raw::c_void) -> ::std::os::raw::c_int;

    /// Retrieves the plugin extension handle used for loading plugins.
    ///
    /// # Returns
    ///
    /// Pointer to the extension handle, or `null` if it failed to load.
    pub fn metacall_plugin_extension() -> *mut ::std::os::raw::c_void;

    /// Retrieves the handle containing all the functionality of the plugins from the core.
    ///
    /// # Returns
    ///
    /// Pointer to the core plugin handle, or `null` if it failed to load.
    pub fn metacall_plugin_core() -> *mut ::std::os::raw::c_void;

    /// Retrieves the plugin extension path used for accessing the plugins folder.
    ///
    /// # Returns
    ///
    /// Pointer to a string containing the core plugin path, or `null` if it failed to load the plugin extension.
    pub fn metacall_plugin_path() -> *const ::std::os::raw::c_char;

    /// Destroys the MetaCall library.
    ///
    /// # Returns
    ///
    /// Returns zero if successful, and a non-zero value otherwise.
    pub fn metacall_destroy() -> ::std::os::raw::c_int;

    /// Converts the specified value into a serialized string.
    ///
    /// # Parameters
    ///
    /// - `name`: Name of the serialization format to be used.
    /// - `v`: Reference to the value to serialize.
    /// - `size`: Output parameter that will contain the size of the newly allocated string.
    /// - `allocator`: Pointer to the allocator that will allocate the string.
    ///
    /// # Returns
    ///
    /// Pointer to a newly allocated string containing the stringified value.
    pub fn metacall_serialize(
        name: *const ::std::os::raw::c_char,
        v: *mut ::std::os::raw::c_void,
        size: *mut usize,
        allocator: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_char;

    /// Converts the specified string buffer into a value.
    ///
    /// # Parameters
    ///
    /// - `name`: Name of the serialization format to be used.
    /// - `buffer`: String to be deserialized.
    /// - `size`: Size of the string buffer.
    /// - `allocator`: Pointer to the allocator that will allocate the value.
    ///
    /// # Returns
    ///
    /// Pointer to a newly allocated value representing the string. This value must be freed when no longer needed.
    pub fn metacall_deserialize(
        name: *const ::std::os::raw::c_char,
        buffer: *const ::std::os::raw::c_char,
        size: usize,
        allocator: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_void;

    /// Retrieves the value contained within a throwable object.
    ///
    /// # Parameters
    ///
    /// - `th`: Pointer to the throwable object.
    ///
    /// # Returns
    ///
    /// Pointer to the value inside the throwable, or `NULL` in case of an error.
    pub fn metacall_throwable_value(th: *mut ::std::os::raw::c_void)
        -> *mut ::std::os::raw::c_void;

    /// Provides information about all loaded objects.
    ///
    /// # Parameters
    ///
    /// - `size`: Size in bytes of the return buffer (output parameter).
    /// - `allocator`: Pointer to the allocator that will allocate the string.
    ///
    /// # Returns
    ///
    /// A string containing introspection information about the loaded objects.
    pub fn metacall_inspect(
        size: *mut usize,
        allocator: *mut ::std::os::raw::c_void,
    ) -> *mut ::std::os::raw::c_char;

    /// Provides information about all loaded objects as a value.
    ///
    /// # Returns
    ///
    /// Pointer to a value containing introspection information.
    pub fn metacall_inspect_value() -> *mut ::std::os::raw::c_void;
}
