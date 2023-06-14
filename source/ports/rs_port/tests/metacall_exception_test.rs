use metacall::{hooks, loaders};
use std::env;

#[test]
fn inlines() {
    let _d = hooks::initialize().unwrap();

    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let js_test_file = tests_dir.join("script.js");

    if let Ok(_) = loaders::from_single_file("node", js_test_file) {
        // This should not generate a segmentation fault
        let val =
            metacall::metacall_no_arg::<metacall::MetacallException>("test_exception").unwrap();

        let cloned_val_1 = val.clone();
        let cloned_val_2 = val.clone();
        let cloned_val_3 = val.clone();

        drop(cloned_val_1);
        drop(cloned_val_2);
        drop(cloned_val_3);

        drop(val);

        // Neither this should not generate a segmentation fault
        let val =
            metacall::metacall_no_arg::<metacall::MetacallException>("test_exception").unwrap();

        let cloned_val_1 = val.clone();
        let cloned_val_2 = val.clone();
        let cloned_val_3 = val.clone();

        drop(val);

        drop(cloned_val_1);
        drop(cloned_val_2);
        drop(cloned_val_3);
    }
}
