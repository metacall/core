use crate::{ c_void, c_char, CStr, c_int };

#[no_mangle]
pub extern "C" fn rs_loader_impl_execution_path(_loader_impl: *mut c_void, path: *const c_char) -> c_int {
	let c_path: &CStr = unsafe { CStr::from_ptr(path) };
	let path_slice: &str = c_path.to_str().unwrap();
	println!("RUST: {}", path_slice);
	0 as c_int
}
