use crate::{
    bindings::metacall_await_future,
    parsers::{any_to_metacall, metacall_to_any},
};
use std::{ffi::c_void, ptr, sync::Arc};

use super::Any;

#[derive(Clone, Hash)]
pub struct MetacallFuture {
    future: Arc<*mut c_void>,
}
unsafe impl Send for MetacallFuture {}
unsafe impl Sync for MetacallFuture {}

pub type MetacallFutureResolve = unsafe extern "C" fn(Any, Any) -> Any;
pub type MetacallFutureReject = unsafe extern "C" fn(Any, Any) -> Any;
type MetacallFutureHandlersTuple = (
    Option<MetacallFutureResolve>,
    Option<MetacallFutureReject>,
    Option<Any>,
);

impl MetacallFuture {
    pub fn new(future: *mut c_void) -> Self {
        Self {
            future: Arc::new(future),
        }
    }

    pub fn await_fut(
        &self,
        resolve: Option<MetacallFutureResolve>,
        reject: Option<MetacallFutureReject>,
        data: Option<Any>,
    ) -> Any {
        let data = if let Some(data) = data {
            any_to_metacall([data])[0]
        } else {
            ptr::null_mut()
        };

        let handlers = &mut (resolve, reject, data) as *mut _ as *mut c_void;

        unsafe extern "C" fn resolver(
            resolve_data: *mut c_void,
            upper_data: *mut c_void,
        ) -> *mut c_void {
            let handlers: *mut MetacallFutureHandlersTuple = upper_data.cast();
            let handlers: MetacallFutureHandlersTuple = ptr::read(handlers);

            if let Some(resolve) = handlers.0 {
                let data = if let Some(data) = handlers.2 {
                    data
                } else {
                    Any::Null
                };

                any_to_metacall([resolve(metacall_to_any(resolve_data), data)])[0]
            } else {
                ptr::null_mut()
            }
        }
        unsafe extern "C" fn rejecter(
            reject_data: *mut c_void,
            upper_data: *mut c_void,
        ) -> *mut c_void {
            let handlers: *mut MetacallFutureHandlersTuple = upper_data.cast();
            let handlers: MetacallFutureHandlersTuple = ptr::read(handlers);

            if let Some(reject) = handlers.1 {
                let data = if let Some(data) = handlers.2 {
                    data
                } else {
                    Any::Null
                };

                any_to_metacall([reject(metacall_to_any(reject_data), data)])[0]
            } else {
                ptr::null_mut()
            }
        }

        metacall_to_any(unsafe {
            metacall_await_future(*self.future, Some(resolver), Some(rejecter), handlers)
        })
    }

    pub fn into_raw(self) -> *mut c_void {
        *self.future
    }
}
