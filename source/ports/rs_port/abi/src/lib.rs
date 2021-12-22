pub mod interface {
    use std::os::raw::{c_char, c_double, c_float, c_int, c_long, c_short, c_void};

    // requires libmetacall to be in $PATH
    #[link(name = "metacall")]
    extern "C" {
        pub fn metacall_destroy() -> c_int;
        pub fn metacall_initialize() -> c_int;
        pub fn metacall_value_destroy(v: *mut c_void);
        pub fn metacall_value_id(v: *mut c_void) -> c_int;
        pub fn metacall_value_to_int(v: *mut c_void) -> c_int;
        pub fn metacall_value_to_bool(v: *mut c_void) -> c_int;
        pub fn metacall_value_to_char(v: *mut c_void) -> c_char;
        pub fn metacall_value_to_long(v: *mut c_void) -> c_long;
        pub fn metacall_value_to_short(v: *mut c_void) -> c_short;
        pub fn metacall_value_to_float(v: *mut c_void) -> c_float;
        pub fn metacall_value_create_int(i: c_int) -> *mut c_void;
        pub fn metacall_value_create_bool(b: c_int) -> *mut c_void;
        pub fn metacall_value_create_long(l: c_long) -> *mut c_void;
        pub fn metacall_value_to_double(v: *mut c_void) -> c_double;
        pub fn metacall_function(cfn: *const c_char) -> *mut c_void;
        pub fn metacall_value_create_char(st: c_char) -> *mut c_void;
        pub fn metacall_value_create_short(s: c_short) -> *mut c_void;
        pub fn metacall_value_create_float(f: c_float) -> *mut c_void;
        pub fn metacall_value_to_string(v: *mut c_void) -> *mut c_char;
        pub fn metacall_value_create_double(d: c_double) -> *mut c_void;
        pub fn metacall_value_create_string(st: *const c_char, ln: usize) -> *mut c_void;
        pub fn metacallfv_s(func: *mut c_void, args: *mut *mut c_void, size: usize) -> *mut c_void;
        pub fn metacall_load_from_file(
            tag: *const c_char,
            paths: *mut *const u8,
            size: usize,
            handle: *mut *mut c_void,
        ) -> c_int;
        pub fn metacall_load_from_memory(
            tag: *const c_char,
            buffer: *const c_char,
            size: usize,
            handle: *mut *mut c_void,
        ) -> c_int;
    }
}
