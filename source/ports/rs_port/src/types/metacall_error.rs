use super::MetaCallValue;
use std::{ffi::NulError, path::PathBuf};

#[derive(Debug, Clone)]
/// This error happens when it's not possible to initialize the MetaCall core. You can check
/// your logs for more information.
pub struct MetaCallInitError;
impl MetaCallInitError {
    #[doc(hidden)]
    pub fn new() -> Self {
        Self
    }
}
impl Default for MetaCallInitError {
    fn default() -> Self {
        MetaCallInitError::new()
    }
}
impl ToString for MetaCallInitError {
    fn to_string(&self) -> String {
        String::from("Failed to initialize MetaCall!")
    }
}

#[derive(Debug, Clone)]
/// This error may happen when passing contains a null character. You can access the
/// original string and the NulError throughout this struct.
pub struct MetaCallStringConversionError {
    pub original_string: String,
    pub nul_error: NulError,
}
impl MetaCallStringConversionError {
    #[doc(hidden)]
    pub fn new(original_string: impl ToString, nul_error: NulError) -> Self {
        Self {
            original_string: original_string.to_string(),
            nul_error,
        }
    }
}
impl ToString for MetaCallStringConversionError {
    fn to_string(&self) -> String {
        self.original_string.clone()
    }
}

#[derive(Debug, Clone)]
/// This error may happen when trying to call a function.
pub enum MetaCallError {
    /// Function not found.
    FunctionNotFound,
    /// Failed to cast the return type as the type requested.
    FailedCasting(Box<dyn MetaCallValue>),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetaCallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to set a class/object attribute. Check your logs
/// if you get `SetAttributeFailure` error variant.
pub enum MetaCallSetAttributeError {
    /// Failed to set the attribute.
    SetAttributeFailure,
    /// Null character detected.
    UnexpectedCStringConversionErr(MetaCallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to get a class/object attribute.
pub enum MetaCallGetAttributeError {
    /// Failed to cast the attribute as the type requested.
    FailedCasting(Box<dyn MetaCallValue>),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetaCallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when loading a code. Check your logs for more information if you
/// get `FromFileFailure` or `FromMemoryFailure` error variant.
pub enum MetaCallLoaderError {
    /// File not found.
    FileNotFound(PathBuf),
    /// Failed to load from file.
    FromFileFailure,
    /// Failed to load from memory.
    FromMemoryFailure,
    /// Not a file or permission denied.
    NotAFileOrPermissionDenied(PathBuf),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetaCallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to get a class by its name.
pub enum MetaCallClassFromNameError {
    /// Class not found.
    ClassNotFound,
    /// Null character detected.
    UnexpectedCStringConversionErr(MetaCallStringConversionError),
}
