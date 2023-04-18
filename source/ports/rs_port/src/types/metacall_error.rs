use super::MetacallValue;
use std::{ffi::NulError, path::PathBuf};

#[derive(Debug, Clone)]
/// This error happens when it's not possible to initialize the Metacall core. You can check
/// your logs for more information.
pub struct MetacallInitError;
impl MetacallInitError {
    #[doc(hidden)]
    pub fn new() -> Self {
        Self
    }
}
impl Default for MetacallInitError {
    fn default() -> Self {
        MetacallInitError::new()
    }
}
impl ToString for MetacallInitError {
    fn to_string(&self) -> String {
        String::from("Failed to initialize Metacall!")
    }
}

#[derive(Debug, Clone)]
/// This error may happen when passing contains a null character. You can access the
/// original string and the NulError throughout this struct.
pub struct MetacallStringConversionError {
    pub original_string: String,
    pub nul_error: NulError,
}
impl MetacallStringConversionError {
    #[doc(hidden)]
    pub fn new(original_string: impl ToString, nul_error: NulError) -> Self {
        Self {
            original_string: original_string.to_string(),
            nul_error,
        }
    }
}
impl ToString for MetacallStringConversionError {
    fn to_string(&self) -> String {
        self.original_string.clone()
    }
}

#[derive(Debug, Clone)]
/// This error may happen when trying to call a function.
pub enum MetacallError {
    /// Function not found.
    FunctionNotFound,
    /// Failed to cast the return type as the type requested.
    FailedCasting(Box<dyn MetacallValue>),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to set a class/object attribute. Check your logs
/// if you get `SetAttributeFailure` error variant.
pub enum MetacallSetAttributeError {
    /// Failed to set the attribute.
    SetAttributeFailure,
    /// Null character detected.
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to get a class/object attribute.
pub enum MetacallGetAttributeError {
    /// Failed to cast the attribute as the type requested.
    FailedCasting(Box<dyn MetacallValue>),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when loading a code. Check your logs for more information if you
/// get `FromFileFailure` or `FromMemoryFailure` error variant.
pub enum MetacallLoaderError {
    /// File not found.
    FileNotFound(PathBuf),
    /// Failed to load from file.
    FromFileFailure,
    /// Failed to load from memory.
    FromMemoryFailure,
    /// Not a file or permission denied.
    NotAFileOrPermissionDenied(PathBuf),
    /// Null character detected.
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
/// This error may happen when trying to get a class by its name.
pub enum MetacallClassFromNameError {
    /// Class not found.
    ClassNotFound,
    /// Null character detected.
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}
