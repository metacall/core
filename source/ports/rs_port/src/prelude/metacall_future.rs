use super::{MetacallNull, MetacallObjectProtocol};
use crate::{
    bindings::{metacall_await_future, metacall_value_destroy, metacall_value_to_future},
    parsers,
};
use std::{ffi::c_void, ptr, sync::Arc};

#[derive(Clone, Debug)]
pub struct MetacallFuture {
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallFuture {}
unsafe impl Sync for MetacallFuture {}

pub type MetacallFutureResolve =
    fn(Box<dyn MetacallObjectProtocol>, Box<dyn MetacallObjectProtocol>);
pub type MetacallFutureReject =
    fn(Box<dyn MetacallObjectProtocol>, Box<dyn MetacallObjectProtocol>);

struct MetacallHandlers {
    pub resolve: Option<MetacallFutureResolve>,
    pub reject: Option<MetacallFutureReject>,
    pub user_data: Option<Box<dyn MetacallObjectProtocol>>,
}

unsafe extern "C" fn resolver(resolve_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let handlers: Box<MetacallHandlers> = parsers::pointer_to_box(upper_data);

    if let Some(resolve) = handlers.resolve {
        let data = if let Some(data) = handlers.user_data {
            data
        } else {
            Box::new(MetacallNull())
        };

        resolve(parsers::raw_to_metacallobj_untyped(resolve_data), data);
    }

    ptr::null_mut()
}
unsafe extern "C" fn rejecter(reject_data: *mut c_void, upper_data: *mut c_void) -> *mut c_void {
    let handlers: Box<MetacallHandlers> = parsers::pointer_to_box(upper_data);

    if let Some(reject) = handlers.reject {
        let data = if let Some(data) = handlers.user_data {
            data
        } else {
            Box::new(MetacallNull())
        };

        reject(parsers::raw_to_metacallobj_untyped(reject_data), data);
    }

    ptr::null_mut()
}

impl MetacallFuture {
    pub fn new(ptr: *mut c_void) -> Self {
        Self { ptr: Arc::new(ptr) }
    }

    pub fn await_fut<T: MetacallObjectProtocol>(
        self,
        resolve: Option<MetacallFutureResolve>,
        reject: Option<MetacallFutureReject>,
        user_data: Option<T>,
    ) {
        let future = self.into_raw();
        let user_data = match user_data {
            Some(user_data) => Some(Box::new(user_data) as Box<dyn MetacallObjectProtocol>),
            None => None,
        };
        let handlers = parsers::new_void_pointer(MetacallHandlers {
            resolve,
            reject,
            user_data,
        });

        unsafe {
            match (resolve.is_some(), reject.is_some()) {
                (true, true) => {
                    metacall_await_future(future, Some(resolver), Some(rejecter), handlers)
                }
                (true, false) => metacall_await_future(future, Some(resolver), None, handlers),
                (false, true) => metacall_await_future(future, None, Some(rejecter), handlers),
                (false, false) => metacall_await_future(future, None, None, ptr::null_mut()),
            }
        };
    }
    pub fn await_fut_no_data(
        self,
        resolve: Option<MetacallFutureResolve>,
        reject: Option<MetacallFutureReject>,
    ) {
        self.await_fut(resolve, reject, None::<MetacallNull>)
    }

    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_to_future(*self.ptr) }
    }
}

impl Drop for MetacallFuture {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) };
    }
}
