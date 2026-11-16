use metacall::{
    initialize, is_initialized,
    load::{self, Tag},
    MetaCallException,
};
use std::env;

fn test_exception_new_and_drop() {
    let ex = MetaCallException::new(
        "Invalid state encountered",
        "RuntimeError",
        "at module.rs:42",
        404,
    )
    .expect("MetaCallException::new should succeed");

    assert_eq!(ex.get_message(), "Invalid state encountered");
    assert_eq!(ex.get_label(), "RuntimeError");
    assert_eq!(ex.get_stacktrace(), "at module.rs:42");
    assert_eq!(ex.get_code(), 404);

    drop(ex);
}

fn test_exception_clone_outlives_original() {
    let clone = {
        let original =
            MetaCallException::new("Original error", "CustomException", "at handler.rs:12", 500)
                .expect("Failed to create original exception");

        assert_eq!(original.get_message(), "Original error");
        original.clone()
    }; // original is dropped here

    assert_eq!(clone.get_message(), "Original error");
    assert_eq!(clone.get_label(), "CustomException");
    assert_eq!(clone.get_stacktrace(), "at handler.rs:12");
    assert_eq!(clone.get_code(), 500);

    drop(clone);
}

fn test_exception_original_outlives_clone() {
    let original = MetaCallException::new(
        "Persistent error",
        "PersistentLabel",
        "at service.rs:88",
        503,
    )
    .expect("Failed to create original exception");

    {
        let clone = original.clone();
        assert_eq!(clone.get_message(), "Persistent error");
        assert_eq!(clone.get_label(), "PersistentLabel");
        assert_eq!(clone.get_stacktrace(), "at service.rs:88");
        assert_eq!(clone.get_code(), 503);
    } // clone is dropped here

    assert_eq!(original.get_message(), "Persistent error");
    assert_eq!(original.get_label(), "PersistentLabel");
    assert_eq!(original.get_stacktrace(), "at service.rs:88");
    assert_eq!(original.get_code(), 503);

    drop(original);
}

fn test_exception_repeated_cloning() {
    let original =
        MetaCallException::new("Repeated clone message", "CloneLabel", "at clone.rs:1", 200)
            .expect("Failed to create original exception");

    let mut clones = Vec::new();
    for _ in 0..20 {
        clones.push(original.clone());
    }

    // Drop original while clones are alive
    drop(original);

    for clone in &clones {
        assert_eq!(clone.get_message(), "Repeated clone message");
        assert_eq!(clone.get_label(), "CloneLabel");
        assert_eq!(clone.get_stacktrace(), "at clone.rs:1");
        assert_eq!(clone.get_code(), 200);
    }

    drop(clones);
}

fn test_exception_nodejs_load() {
    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let js_test_file = tests_dir.join("script.js");

    if load::from_single_file(Tag::NodeJS, js_test_file, None).is_ok() {
        // This should not generate a segmentation fault
        let val =
            metacall::metacall_no_arg::<metacall::MetaCallException>("test_exception").unwrap();

        let cloned_val_1 = val.clone();
        let cloned_val_2 = val.clone();
        let cloned_val_3 = val.clone();

        drop(cloned_val_1);
        drop(cloned_val_2);
        drop(cloned_val_3);

        drop(val);

        // Neither this should not generate a segmentation fault
        let val =
            metacall::metacall_no_arg::<metacall::MetaCallException>("test_exception").unwrap();

        let cloned_val_1 = val.clone();
        let cloned_val_2 = val.clone();
        let cloned_val_3 = val.clone();

        drop(val);

        drop(cloned_val_1);
        drop(cloned_val_2);
        drop(cloned_val_3);
    }
}

#[test]
fn metacall_exception() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    test_exception_new_and_drop();
    test_exception_clone_outlives_original();
    test_exception_original_outlives_clone();
    test_exception_repeated_cloning();
    test_exception_nodejs_load();
}
