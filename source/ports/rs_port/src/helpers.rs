use crate::types::MetacallValue;
use std::any::Any;

pub trait MetacallDowncast: Any {
    fn into_any(self: Box<Self>) -> Box<dyn Any>;
    fn as_any(&self) -> &dyn Any;
    fn as_any_mut(&mut self) -> &mut dyn Any;
}
impl<T: Any> MetacallDowncast for T {
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
impl dyn MetacallValue {
    /// Checks if the trait object is having the given type.
    pub fn is<T: MetacallValue>(&self) -> bool {
        MetacallDowncast::as_any(self).is::<T>()
    }

    /// Downcasts the inner value of the trait object and returns the ownership.
    pub fn downcast<T: MetacallValue>(self: Box<Self>) -> Result<T, Box<Self>> {
        if self.is::<T>() {
            Ok(*MetacallDowncast::into_any(self).downcast::<T>().unwrap())
        } else {
            Err(self)
        }
    }

    /// Downcasts the inner value of the trait object and returns a reference.
    pub fn downcast_ref<T: MetacallValue>(&self) -> Option<&T> {
        MetacallDowncast::as_any(self).downcast_ref::<T>()
    }

    /// Downcasts the inner value of the trait object and returns a mutable reference.
    pub fn downcast_mut<T: MetacallValue>(&mut self) -> Option<&mut T> {
        MetacallDowncast::as_any_mut(self).downcast_mut::<T>()
    }
}

pub trait MetacallSealed {}
impl<T: Clone> MetacallSealed for T {}
impl MetacallSealed for str {}
impl<T: Clone> MetacallSealed for [T] {}

pub fn clone_box<T>(t: &T) -> Box<T>
where
    T: ?Sized + MetacallClone,
{
    unsafe {
        let mut fat_ptr = t as *const T;
        let data_ptr = &mut fat_ptr as *mut *const T as *mut *mut ();

        assert_eq!(*data_ptr as *const (), t as *const T as *const ());

        *data_ptr = <T as MetacallClone>::clone_box(t);

        Box::from_raw(fat_ptr as *mut T)
    }
}

pub trait MetacallClone: MetacallSealed {
    fn clone_box(&self) -> *mut ();
}
impl<T> MetacallClone for T
where
    T: Clone,
{
    fn clone_box(&self) -> *mut () {
        Box::<T>::into_raw(Box::new(self.clone())) as *mut ()
    }
}

impl MetacallClone for str {
    fn clone_box(&self) -> *mut () {
        Box::<str>::into_raw(Box::from(self)) as *mut ()
    }
}
impl<T> MetacallClone for [T]
where
    T: Clone,
{
    fn clone_box(&self) -> *mut () {
        Box::<[T]>::into_raw(self.iter().cloned().collect()) as *mut ()
    }
}
impl<'c> Clone for Box<dyn MetacallValue + 'c> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl<'c> Clone for Box<dyn MetacallValue + Send + 'c> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl<'c> Clone for Box<dyn MetacallValue + Sync + 'c> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}
impl<'c> Clone for Box<dyn MetacallValue + Send + Sync + 'c> {
    fn clone(&self) -> Self {
        clone_box(&**self)
    }
}

pub fn metacall_implementer_to_traitobj(v: impl MetacallValue) -> Box<dyn MetacallValue> {
    Box::new(v) as Box<dyn MetacallValue>
}
