use crate::{
    bindings::{
        metacall_allocator_create, metacall_allocator_destroy, metacall_allocator_id,
        metacall_allocator_std_type,
    },
    types::MetaCallAllocatorError,
};
use std::{ffi::c_void, ptr::NonNull};

extern "C" {
    fn malloc(size: usize) -> *mut c_void;
    fn realloc(ptr: *mut c_void, size: usize) -> *mut c_void;
    fn free(ptr: *mut c_void);
}

#[derive(Debug)]
/// MetaCall allocator backed by malloc, realloc and free. It is destroyed when dropped.
pub struct MetaCallAllocator(NonNull<c_void>);

impl MetaCallAllocator {
    pub fn new() -> Result<Self, MetaCallAllocatorError> {
        let mut std_ctx = metacall_allocator_std_type {
            malloc: Some(malloc),
            realloc: Some(realloc),
            free: Some(free),
        };
        // The context is copied by MetaCall, so it can live in the stack
        let allocator = unsafe {
            metacall_allocator_create(
                metacall_allocator_id::METACALL_ALLOCATOR_STD,
                (&mut std_ctx as *mut metacall_allocator_std_type).cast(),
            )
        };

        NonNull::new(allocator)
            .map(Self)
            .ok_or(MetaCallAllocatorError)
    }

    pub(crate) fn as_raw(&self) -> *mut c_void {
        self.0.as_ptr()
    }
}

impl Drop for MetaCallAllocator {
    fn drop(&mut self) {
        unsafe { metacall_allocator_destroy(self.as_raw()) }
    }
}
