#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct MetacallVersionType {
    pub major: ::std::os::raw::c_uint,
    pub minor: ::std::os::raw::c_uint,
    pub patch: ::std::os::raw::c_uint,
    pub revision: *const ::std::os::raw::c_char,
    pub str_: *const ::std::os::raw::c_char,
    pub name: *const ::std::os::raw::c_char,
}


extern "C" {
    /// Provides the module version structure.
    ///
    /// # Returns
    ///
    /// Pointer to a static struct containing the unpacked version information.
    pub fn metacall_version() -> *const MetacallVersionType;

    /// Creates a packed hexadecimal representation of the module version.
    ///
    /// The format is `0xMMIIPPPP`, where:
    /// - `M` is the major version.
    /// - `I` is the minor version.
    /// - `P` is the patch version.
    ///
    /// # Parameters
    ///
    /// - `major`: Unsigned integer representing the major version.
    /// - `minor`: Unsigned integer representing the minor version.
    /// - `patch`: Unsigned integer representing the patch version.
    ///
    /// # Returns
    ///
    /// Hexadecimal integer containing the packed version.
    pub fn metacall_version_hex_make(
        major: ::std::os::raw::c_uint,
        minor: ::std::os::raw::c_uint,
        patch: ::std::os::raw::c_uint,
    ) -> u32;

    /// Provides the packed hexadecimal value of the module version.
    ///
    /// The format is `0xMMIIPPPP`, where:
    /// - `M` is the major version.
    /// - `I` is the minor version.
    /// - `P` is the patch version.
    ///
    /// # Returns
    ///
    /// Hexadecimal integer containing the packed version.
    pub fn metacall_version_hex() -> u32;

    /// Provides the module version as a string.
    ///
    /// # Returns
    ///
    /// Pointer to a static string containing the module version.
    pub fn metacall_version_str() -> *const ::std::os::raw::c_char;

    /// Provides the module version revision as a string.
    ///
    /// # Returns
    ///
    /// Pointer to a static string containing the module version revision.
    pub fn metacall_version_revision() -> *const ::std::os::raw::c_char;

    /// Provides the module version name as a string.
    ///
    /// # Returns
    ///
    /// Pointer to a static string containing the module version name.
    pub fn metacall_version_name() -> *const ::std::os::raw::c_char;

    /// Provides information about the module.
    ///
    /// # Returns
    ///
    /// Pointer to a static string containing module information.
    pub fn metacall_print_info() -> *const ::std::os::raw::c_char;
}
