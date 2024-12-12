use crate::types::MetaCallValue;
use std::any::Any;

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
}

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

pub fn metacall_implementer_to_traitobj(v: impl MetaCallValue) -> Box<dyn MetaCallValue> {
    Box::new(v) as Box<dyn MetaCallValue>
}
