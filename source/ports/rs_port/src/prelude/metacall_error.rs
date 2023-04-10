use super::MetacallObjectProtocol;
use std::{ffi::NulError, path::PathBuf};

#[derive(Debug, Clone)]
pub struct MetacallInitError(pub String);
impl MetacallInitError {
    pub fn new() -> Self {
        Self(String::from("Failed to initialize Metacall!"))
    }
}
impl Default for MetacallInitError {
    fn default() -> Self {
        MetacallInitError::new()
    }
}
impl ToString for MetacallInitError {
    fn to_string(&self) -> String {
        self.0.clone()
    }
}

#[derive(Debug, Clone)]
pub struct MetacallStringConversionError {
    pub original_string: String,
    pub nul_error: NulError,
}
impl MetacallStringConversionError {
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
pub enum MetacallError {
    FunctionNotFound,
    FailedCasting(Box<dyn MetacallObjectProtocol>),
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
pub enum MetacallSetAttributeError {
    SetAttributeFailure,
    FailedCasting(Box<dyn MetacallObjectProtocol>),
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
pub enum MetacallGetAttributeError {
    FailedCasting(Box<dyn MetacallObjectProtocol>),
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}

#[derive(Debug, Clone)]
pub enum MetacallLoaderError {
    FileNotFound(PathBuf),
    FromFileFailure,
    FromMemoryFailure,
    NotAFileOrPermissionDenied(PathBuf),
    UnexpectedCStringConversionErr(MetacallStringConversionError),
}
