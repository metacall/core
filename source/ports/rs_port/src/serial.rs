use crate::{
    allocator::MetaCallAllocator,
    bindings::{
        metacall_allocator_free, metacall_deserialize, metacall_serial, metacall_serialize,
        metacall_value_destroy,
    },
    cast,
    types::{MetaCallSerialError, MetaCallStringConversionError, MetaCallValue},
};
use std::{
    ffi::{CStr, CString},
    ptr,
};

/// Converts a value into a serialized string (JSON by default). For example: ...
/// ```
/// let json = metacall::serialize(vec![1, 2, 3]).unwrap();
/// assert_eq!(json, "[1,2,3]");
/// ```
pub fn serialize(value: impl MetaCallValue) -> Result<String, MetaCallSerialError> {
    let allocator = MetaCallAllocator::new().map_err(MetaCallSerialError::AllocatorFailure)?;
    let raw = value.into_metacall_raw();
    let mut size = 0;
    let buffer =
        unsafe { metacall_serialize(metacall_serial(), raw, &mut size, allocator.as_raw()) };

    unsafe { metacall_value_destroy(raw) };

    if buffer.is_null() {
        return Err(MetaCallSerialError::SerializationFailure);
    }

    let result = unsafe { CStr::from_ptr(buffer) }
        .to_string_lossy()
        .into_owned();

    unsafe { metacall_allocator_free(allocator.as_raw(), buffer.cast()) };

    Ok(result)
}

/// Converts a serialized string (JSON by default) into a value. For example: ...
/// ```
/// let value = metacall::deserialize::<Box<dyn metacall::MetaCallValue>>(r#"{ "a": [1, 2] }"#).unwrap();
/// assert_eq!(value["a"][1], 2);
/// ```
pub fn deserialize<T: MetaCallValue>(buffer: impl AsRef<str>) -> Result<T, MetaCallSerialError> {
    let buffer = buffer.as_ref();
    let c_buffer = CString::new(buffer).map_err(|err| {
        MetaCallSerialError::UnexpectedCStringConversionErr(MetaCallStringConversionError::new(
            buffer, err,
        ))
    })?;
    // The size must include the null terminator, the allocator is not used when deserializing
    let size = c_buffer.as_bytes_with_nul().len();
    let raw = unsafe {
        metacall_deserialize(metacall_serial(), c_buffer.as_ptr(), size, ptr::null_mut())
    };

    if raw.is_null() {
        return Err(MetaCallSerialError::DeserializationFailure);
    }

    cast::raw_to_metacallobj::<T>(raw).map_err(MetaCallSerialError::FailedCasting)
}
