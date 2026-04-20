/// A macro to check if a raw pointer is null and return a custom error.
#[macro_export]
macro_rules! check_null_ptr {
    ($ptr:expr, $err:expr) => {
        if $ptr.is_null() {
            return Err($err.into());
        }
    };
}
