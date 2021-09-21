use crate::{ c_void, c_int};

#[no_mangle]
pub extern "C" fn rs_loader_impl_initialize(_loader_impl: *mut c_void, _config: *mut c_void) -> *mut c_void {
	1 as *const c_int as *mut c_void
}
