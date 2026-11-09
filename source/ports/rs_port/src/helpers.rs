use crate::types::MetaCallValue;
use std::{any::Any, collections::HashMap, ops::Index};

pub trait MetaCallDowncast: Any {
    fn into_any(self: Box<Self>) -> Box<dyn Any>;
    fn as_any(&self) -> &dyn Any;
    fn as_any_mut(&mut self) -> &mut dyn Any;
}
impl<T: Any> MetaCallDowncast for T {
    fn into_any(self: Box<Self>) -> Box<dyn Any> {
        self
    }
    fn as_any(&self) -> &dyn Any {
        self
    }
    fn as_any_mut(&mut self) -> &mut dyn Any {
        self
    }
}
impl dyn MetaCallValue {
    /// Checks if the trait object is having the given type.
    pub fn is<T: MetaCallValue>(&self) -> bool {
        MetaCallDowncast::as_any(self).is::<T>()
    }

    /// Downcasts the inner value of the trait object and returns the ownership.
    pub fn downcast<T: MetaCallValue>(self: Box<Self>) -> Result<T, Box<Self>> {
        if self.is::<T>() {
            Ok(*MetaCallDowncast::into_any(self).downcast::<T>().unwrap())
        } else {
            Err(self)
        }
    }

    /// Downcasts the inner value of the trait object and returns a reference.
    pub fn downcast_ref<T: MetaCallValue>(&self) -> Option<&T> {
        MetaCallDowncast::as_any(self).downcast_ref::<T>()
    }

    /// Downcasts the inner value of the trait object and returns a mutable reference.
    pub fn downcast_mut<T: MetaCallValue>(&mut self) -> Option<&mut T> {
        MetaCallDowncast::as_any_mut(self).downcast_mut::<T>()
    }

    /// Returns the string if the value is a string.
    pub fn as_str(&self) -> Option<&str> {
        self.downcast_ref::<String>().map(String::as_str)
    }

    /// Returns the map if the value is a map.
    pub fn as_map(&self) -> Option<&HashMap<String, Box<dyn MetaCallValue>>> {
        self.downcast_ref::<HashMap<String, Box<dyn MetaCallValue>>>()
    }

    /// Returns the array if the value is an array.
    pub fn as_array(&self) -> Option<&Vec<Box<dyn MetaCallValue>>> {
        self.downcast_ref::<Vec<Box<dyn MetaCallValue>>>()
    }
}

/// Access a map by key, panics if the value is not a map or the key does not exist.
impl Index<&str> for dyn MetaCallValue {
    type Output = dyn MetaCallValue;

    fn index(&self, key: &str) -> &Self::Output {
        let map = self.as_map().expect("MetaCall value is not a map");

        &**map.get(key).expect("MetaCall map key not found")
    }
}

/// Access an array by position, panics if the value is not an array or out of bounds.
impl Index<usize> for dyn MetaCallValue {
    type Output = dyn MetaCallValue;

    fn index(&self, index: usize) -> &Self::Output {
        &*self.as_array().expect("MetaCall value is not an array")[index]
    }
}

/// Compare a value with a string, false if the value is not a string.
impl PartialEq<str> for dyn MetaCallValue {
    fn eq(&self, other: &str) -> bool {
        self.as_str() == Some(other)
    }
}
impl PartialEq<&str> for dyn MetaCallValue {
    fn eq(&self, other: &&str) -> bool {
        self.as_str() == Some(*other)
    }
}

/// Compare a value with a primitive, false if the value is not of the same type.
macro_rules! impl_partial_eq_primitive {
    ($($t:ty),*) => {$(
        impl PartialEq<$t> for dyn MetaCallValue {
            fn eq(&self, other: &$t) -> bool {
                self.downcast_ref::<$t>() == Some(other)
            }
        }
    )*};
}
impl_partial_eq_primitive!(bool, char, i16, i32, i64, f32, f64, String);

pub trait MetaCallSealed {}
impl<T: Clone> MetaCallSealed for T {}
impl MetaCallSealed for str {}
impl<T: Clone> MetaCallSealed for [T] {}

pub fn clone_box<T>(t: &T) -> Box<T>
where
    T: ?Sized + MetaCallClone,
{
    unsafe {
        let mut fat_ptr = t as *const T;
        let data_ptr = &mut fat_ptr as *mut *const T as *mut *mut ();

        assert_eq!(*data_ptr as *const (), t as *const T as *const ());

        *data_ptr = <T as MetaCallClone>::clone_box(t);

        Box::from_raw(fat_ptr as *mut T)
    }
}

pub trait MetaCallClone: MetaCallSealed {
    fn clone_box(&self) -> *mut ();
}
impl<T> MetaCallClone for T
where
    T: Clone,
{
    fn clone_box(&self) -> *mut () {
        Box::<T>::into_raw(Box::new(self.clone())) as *mut ()
    }
}

impl MetaCallClone for str {
    fn clone_box(&self) -> *mut () {
        Box::<str>::into_raw(Box::from(self)) as *mut ()
    }
}
impl<T> MetaCallClone for [T]
where
    T: Clone,
{
    fn clone_box(&self) -> *mut () {
        Box::<[T]>::into_raw(self.iter().cloned().collect()) as *mut ()
    }
}
impl Clone for Box<dyn MetaCallValue + '_> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl Clone for Box<dyn MetaCallValue + Send + '_> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl Clone for Box<dyn MetaCallValue + Sync + '_> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl Clone for Box<dyn MetaCallValue + Send + Sync + '_> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
