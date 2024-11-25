use metacall_bindings::value::{
    create::metacall_value_create_long, metacall_value_destroy, metacall_value_from_long,
    metacall_value_to_long,
};
use thiserror::Error;
}

#[derive(Error, Debug)]
pub enum ValueError {
    #[error("Failed to create CString: {0}")]
    CStringError(#[from] std::ffi::NulError),
    #[error("Failed to convert CString to &str: {0}")]
    Utf8Error(#[from] std::str::Utf8Error),
    #[error("Null pointer encountered")]
    NullPointer,
    #[error("Unknown error")]
    Unknown,
}
}

impl To<i64> for Value {
    fn to(&self) -> i64 {
        unsafe { metacall_value_to_long(self.0) as i64 }
    }
}

impl Drop for Value {
    fn drop(&mut self) {
        unsafe {
            metacall_value_destroy(self.0);
        }
    }
}

#[cfg(test)]
mod test {
    // use metacall_bindings::{metacall_destroy, metacall_initialize};

    use super::{Create, From, To, Value};

    #[test]
    fn metacall_create_value() {
        // assert!(unsafe { metacall_initialize() } == 0);
        let mut val = Value::new(123);
        let result = val.to();
        assert!(result == 123);
        val.from(33);

        let result = val.to();
        println!("{}", result); // 33
        assert!(result == 33);
        // unsafe { metacall_destroy() };
    }
}
