use metacall::{
    initialize, is_initialized,
    load::{self, Tag},
};
use std::env;

#[test]
fn metacall_exception() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

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
