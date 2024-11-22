use metacall_bindings::value::{
    create::metacall_value_create_long, metacall_value_destroy, metacall_value_from_long,
    metacall_value_to_long,
};
use std::ffi::c_long;

trait Create<T> {
    fn new(value: T) -> Self;
}

trait From<T> {
    fn from(&mut self, value: T) -> &Self;
}

trait To<T> {
    fn to(&self) -> T;
}

struct Value(*mut std::os::raw::c_void);

impl Create<i64> for Value {
    fn new(value: i64) -> Self {
        let val = unsafe { metacall_value_create_long(value as c_long) };
        Self(val)
    }
}

impl From<i64> for Value {
    fn from(&mut self, value: i64) -> &Self {
        self.0 = unsafe { metacall_value_from_long(self.0, value as c_long) };
        self
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
