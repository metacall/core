use metacall::{
    bindings::{
        metacall_value_count, metacall_value_create_int, metacall_value_create_string,
        metacall_value_destroy, metacall_value_id, metacall_value_to_array, metacall_value_to_int,
        metacall_value_to_map, metacall_value_to_string,
    },
    initialize, is_initialized,
    serial::{
        deserialize_from_string, serialize_to_string, MetaCallAllocator, MetaCallSerialError,
    },
};
use std::ffi::CStr;
use std::slice;

#[test]
fn serial() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    test_allocator_creation();
    test_serialize_int();
    test_serialize_string();
    test_deserialize_object();
    test_deserialize_array();
    test_serialize_deserialize_roundtrip();
    test_deserialize_invalid_json();
    test_deserialize_with_explicit_serial();
}

fn test_allocator_creation() {
    let allocator = MetaCallAllocator::new();
    assert!(allocator.is_ok(), "Allocator creation should succeed");
}

fn test_serialize_int() {
    let allocator = MetaCallAllocator::new().unwrap();
    let value = unsafe { metacall_value_create_int(42) };

    let json = serialize_to_string(value, &allocator, None).unwrap();
    assert_eq!(json, "42");

    unsafe { metacall_value_destroy(value) };
}

fn test_serialize_string() {
    let allocator = MetaCallAllocator::new().unwrap();
    let s = "hello world";
    let value = unsafe { metacall_value_create_string(s.as_ptr() as *const i8, s.len()) };

    let json = serialize_to_string(value, &allocator, None).unwrap();
    assert_eq!(json, "\"hello world\"");

    unsafe { metacall_value_destroy(value) };
}

fn test_deserialize_object() {
    let value = deserialize_from_string(r#"{"key":"value","num":42}"#, None).unwrap();
    assert!(!value.is_null());

    unsafe {
        // Verify type is Map
        let type_id = metacall_value_id(value);
        assert_eq!(type_id, metacall_value_id::METACALL_MAP);

        // Verify entry count
        let count = metacall_value_count(value);
        assert_eq!(count, 2, "Map should have 2 entries");

        // Verify each key-value pair
        let pairs = slice::from_raw_parts(metacall_value_to_map(value), count);

        let mut found_key = false;
        let mut found_num = false;

        for &pair_ptr in pairs {
            let pair = slice::from_raw_parts(metacall_value_to_array(pair_ptr), 2);
            let key_str = CStr::from_ptr(metacall_value_to_string(pair[0]))
                .to_str()
                .unwrap();

            match key_str {
                "key" => {
                    let val = CStr::from_ptr(metacall_value_to_string(pair[1]))
                        .to_str()
                        .unwrap();
                    assert_eq!(val, "value");
                    found_key = true;
                }
                "num" => {
                    let val = metacall_value_to_int(pair[1]);
                    assert_eq!(val, 42);
                    found_num = true;
                }
                other => panic!("Unexpected key: {}", other),
            }
        }

        assert!(found_key, "Should have found 'key' entry");
        assert!(found_num, "Should have found 'num' entry");

        metacall_value_destroy(value);
    }
}

fn test_deserialize_array() {
    let value = deserialize_from_string(r#"[1, 2, 3]"#, None).unwrap();
    assert!(!value.is_null());

    unsafe {
        // Verify type is Array
        let type_id = metacall_value_id(value);
        assert_eq!(type_id, metacall_value_id::METACALL_ARRAY);

        // Verify element count
        let count = metacall_value_count(value);
        assert_eq!(count, 3, "Array should have 3 elements");

        // Verify each element value
        let elements = slice::from_raw_parts(metacall_value_to_array(value), count);
        assert_eq!(metacall_value_to_int(elements[0]), 1);
        assert_eq!(metacall_value_to_int(elements[1]), 2);
        assert_eq!(metacall_value_to_int(elements[2]), 3);

        metacall_value_destroy(value);
    }
}

fn test_serialize_deserialize_roundtrip() {
    let allocator = MetaCallAllocator::new().unwrap();

    let original_json = r#"{"name":"metacall","version":1}"#;
    let value = deserialize_from_string(original_json, None).unwrap();

    // Serialize back and verify content is preserved
    let result_json = serialize_to_string(value, &allocator, None).unwrap();
    assert!(result_json.contains("\"name\":\"metacall\""));
    assert!(result_json.contains("\"version\":1"));

    unsafe { metacall_value_destroy(value) };
}

fn test_deserialize_invalid_json() {
    let result = deserialize_from_string("not valid json {{{", None);
    assert!(
        matches!(result, Err(MetaCallSerialError::DeserializationFailed)),
        "Invalid JSON should return DeserializationFailed"
    );
}

fn test_deserialize_with_explicit_serial() {
    let value = deserialize_from_string(r#"{"key":"value"}"#, Some("rapid_json")).unwrap();
    assert!(!value.is_null());

    unsafe {
        // Verify type, count, and content
        assert_eq!(metacall_value_id(value), metacall_value_id::METACALL_MAP);
        assert_eq!(metacall_value_count(value), 1);

        let pairs = slice::from_raw_parts(metacall_value_to_map(value), 1);
        let pair = slice::from_raw_parts(metacall_value_to_array(pairs[0]), 2);

        let key = CStr::from_ptr(metacall_value_to_string(pair[0]))
            .to_str()
            .unwrap();
        let val = CStr::from_ptr(metacall_value_to_string(pair[1]))
            .to_str()
            .unwrap();

        assert_eq!(key, "key");
        assert_eq!(val, "value");

        metacall_value_destroy(value);
    }
}
