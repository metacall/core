use crate::bindings::{
    metacall_allocator_create, metacall_allocator_destroy, metacall_allocator_free,
    metacall_allocator_id, metacall_deserialize, metacall_serial, metacall_serialize,
};
use std::ffi::{c_void, CStr, CString};
use std::fmt;

// ---------------------------------------------------------------------------
// Error type
// ---------------------------------------------------------------------------

/// Errors that can occur during MetaCall serialization operations.
#[derive(Debug, Clone)]
pub enum MetaCallSerialError {
    /// Failed to create the allocator.
    AllocatorCreationFailed,
    /// Failed to serialize the value.
    SerializationFailed,
    /// Failed to deserialize the string.
    DeserializationFailed,
    /// The serialized output contained invalid UTF-8.
    InvalidUtf8,
    /// The input string contained a null byte and could not be
    /// converted to a C string.
    NulInString,
}

impl fmt::Display for MetaCallSerialError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::AllocatorCreationFailed => write!(f, "Failed to create MetaCall allocator"),
            Self::SerializationFailed => write!(f, "MetaCall serialization returned null"),
            Self::DeserializationFailed => write!(f, "MetaCall deserialization returned null"),
            Self::InvalidUtf8 => write!(f, "Serialized output is not valid UTF-8"),
            Self::NulInString => write!(f, "Input string contains an interior null byte"),
        }
    }
}

impl std::error::Error for MetaCallSerialError {}

// ---------------------------------------------------------------------------
// Allocator context (mirrors the C struct exactly)
// ---------------------------------------------------------------------------

#[repr(C)]
struct MetaCallAllocatorStd {
    malloc: unsafe extern "C" fn(usize) -> *mut c_void,
    realloc: unsafe extern "C" fn(*mut c_void, usize) -> *mut c_void,
    free: unsafe extern "C" fn(*mut c_void),
}

extern "C" {
    fn malloc(size: usize) -> *mut c_void;
    fn realloc(ptr: *mut c_void, size: usize) -> *mut c_void;
    fn free(ptr: *mut c_void);
}

// ---------------------------------------------------------------------------
// MetaCallAllocator — RAII wrapper
// ---------------------------------------------------------------------------

/// A reusable allocator for MetaCall serialization operations.
///
/// Create one allocator and pass it to [`serialize_to_string`] as many
/// times as needed.  The underlying C allocator is destroyed automatically
/// when this value is dropped.
///
/// # Example
///
/// ```no_run
/// use metacall::serial::{MetaCallAllocator, serialize_to_string};
///
/// let allocator = MetaCallAllocator::new().unwrap();
///
/// // Use `allocator` for multiple serialize calls…
/// ```
pub struct MetaCallAllocator {
    ptr: *mut c_void,
    /// Prevent the context from being dropped while the allocator is alive.
    _ctx: Box<MetaCallAllocatorStd>,
}

unsafe impl Send for MetaCallAllocator {}

impl MetaCallAllocator {
    /// Create a new standard allocator backed by the C `malloc`/`realloc`/`free`.
    pub fn new() -> Result<Self, MetaCallSerialError> {
        let ctx = Box::new(MetaCallAllocatorStd {
            malloc,
            realloc,
            free,
        });
        let ptr = unsafe {
            metacall_allocator_create(
                metacall_allocator_id::METACALL_ALLOCATOR_STD,
                &*ctx as *const MetaCallAllocatorStd as *mut c_void,
            )
        };
        if ptr.is_null() {
            return Err(MetaCallSerialError::AllocatorCreationFailed);
        }
        Ok(Self { ptr, _ctx: ctx })
    }

    /// Returns the raw allocator pointer for use in FFI calls.
    pub fn as_ptr(&self) -> *mut c_void {
        self.ptr
    }
}

impl Drop for MetaCallAllocator {
    fn drop(&mut self) {
        if !self.ptr.is_null() {
            unsafe { metacall_allocator_destroy(self.ptr) };
        }
    }
}

// ---------------------------------------------------------------------------
// serialize_to_string
// ---------------------------------------------------------------------------

/// Serialize a MetaCall value into a string.
///
/// The caller must provide a reusable [`MetaCallAllocator`].  The serialized
/// buffer is copied into a Rust `String` and then freed; the allocator itself
/// remains valid for future calls.
///
/// # Arguments
///
/// * `value` — A valid MetaCall value pointer (e.g. from `metacallfv_s` or
///   `MetaCallValue::into_metacall_raw`).
/// * `allocator` — A reusable allocator created with [`MetaCallAllocator::new`].
/// * `serial` — The serialization format to use (e.g. `"rapid_json"`).
///   Pass `None` to use the default format configured at MetaCall initialization.
///
/// # Safety
///
/// `value` must be a valid, non-null MetaCall value pointer.
///
/// # Example
///
/// ```no_run
/// use metacall::serial::{MetaCallAllocator, serialize_to_string};
///
/// let allocator = MetaCallAllocator::new().unwrap();
/// // Assuming `ret` is a raw value pointer returned by a MetaCall function call:
/// // let json = serialize_to_string(ret, &allocator, None).unwrap();
/// ```
pub fn serialize_to_string(
    value: *mut c_void,
    allocator: &MetaCallAllocator,
    serial: Option<&str>,
) -> Result<String, MetaCallSerialError> {
    let c_serial = serial.map(|s| CString::new(s).unwrap());
    let serial_ptr = match &c_serial {
        Some(s) => s.as_ptr(),
        None => unsafe { metacall_serial() },
    };

    let mut size: usize = 0;
    let buf = unsafe { metacall_serialize(serial_ptr, value, &mut size, allocator.as_ptr()) };
    if buf.is_null() {
        return Err(MetaCallSerialError::SerializationFailed);
    }

    // The buffer was allocated via malloc through our allocator, copy it into
    // a Rust String and then free the C buffer.
    let result = unsafe {
        CStr::from_ptr(buf)
            .to_str()
            .map(|s| s.to_owned())
            .map_err(|_| MetaCallSerialError::InvalidUtf8)
    };

    // Free the serialized buffer through the allocator.
    unsafe { metacall_allocator_free(allocator.as_ptr(), buf as *mut c_void) };

    result
}

// ---------------------------------------------------------------------------
// deserialize_from_string
// ---------------------------------------------------------------------------

/// Deserialize a string into a MetaCall value.
///
/// Returns a raw value pointer.  **The caller must call
/// `metacall_value_destroy` on the returned pointer when done.**
///
/// # Arguments
///
/// * `json` — A valid serialized string to be deserialized.
/// * `serial` — The serialization format to use (e.g. `"rapid_json"`).
///   Pass `None` to use the default format configured at MetaCall initialization.
///
/// # Example
///
/// ```no_run
/// use metacall::serial::deserialize_from_string;
/// use metacall::bindings::metacall_value_destroy;
///
/// let value = deserialize_from_string(r#"{"key": "value"}"#, None).unwrap();
/// // Use `value` as an argument to metacallfv_s, etc.
/// // ...
/// unsafe { metacall_value_destroy(value) };
/// ```
pub fn deserialize_from_string(
    json: &str,
    serial: Option<&str>,
) -> Result<*mut c_void, MetaCallSerialError> {
    let c_serial = serial.map(|s| CString::new(s).unwrap());
    let serial_ptr = match &c_serial {
        Some(s) => s.as_ptr(),
        None => unsafe { metacall_serial() },
    };

    let c_json = CString::new(json).map_err(|_| MetaCallSerialError::NulInString)?;
    let value = unsafe {
        metacall_deserialize(
            serial_ptr,
            c_json.as_ptr(),
            c_json.as_bytes_with_nul().len(),
            std::ptr::null_mut(),
        )
    };
    if value.is_null() {
        return Err(MetaCallSerialError::DeserializationFailed);
    }
    Ok(value)
}
