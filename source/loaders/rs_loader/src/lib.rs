/*************************************************************************/
/*  lib.rs                                                               */
/*************************************************************************/
/* Copyright (c) 2020-2020 Swarnim Arun.                                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

mod defer;
use defer::defer;

pub mod metacall {
	use std::ffi::CString;
	use std::os::raw::{
		c_int,
		c_short,
		c_char,
		c_void,
		c_float,
		c_double
	};

	#[link(name = "metacall")] // requires libmetacall to be in $PATH
	extern "C" {
		fn metacall_initialize() -> c_int;
		fn metacall_load_from_file(tag : *const c_char,  paths : *mut *const u8, size : usize, handle : *mut *mut c_void) -> c_int;
		fn metacall_function(cfn : *const c_char) -> *mut c_void;
		fn metacall_destroy() -> c_int;
		fn metacallfv(func : *mut c_void, args : *mut *mut c_void) -> *mut c_void;
		fn metacall_value_create_int(i : c_int) -> *mut c_void;
		fn metacall_value_create_double(d : c_double) -> *mut c_void;
		fn metacall_value_create_bool(b : c_int) -> *mut c_void;
		fn metacall_value_create_string(st: *const c_char, ln : usize) -> *mut c_void;
		fn metacall_value_create_char(st: c_char) -> *mut c_void;
		fn metacall_value_destroy(v : *mut c_void);
		fn metacall_value_id(v : *mut c_void) -> c_int;
		fn metacall_value_to_string(v: *mut c_void) -> *mut c_char;
		fn metacall_value_to_int(v: *mut c_void) -> c_int;
		fn metacall_value_to_bool(v: *mut c_void) -> c_int;
		fn metacall_value_to_double(v: *mut c_void) -> c_double;
	}

	#[derive(Debug)]
	pub struct Error(String);

	/// Enum of all possible Metcall types to allow for safe conversion between them and c_types
	#[derive(Debug)]
	pub enum Any {
		Null, // from c_null
		Short(i16), // from c_short
		Int(i32), // from c_int
		Long(i64), // from c_long
		Float(f32), // from c_float
		Double(f64), // from c_double
		Bool(bool), // from c_bool
		Char(char), // from c_char
		Str(String), // from *const u8 (null terminated)
		Array(Vec<Any>), // from *mut *mut c_void
		Buffer(Vec<u8>), // from *const u8 (non-null terminated) (raw binary data)
		Pointer(Box<Any>), // from *mut c_void
		Function(Box<fn(Any) -> Any>) // from a C function pointer
		// METACALL_FUTURE
	}
	impl From<c_short> for Any {
		fn from(val : c_short) -> Self { Any::Short(val) }
	}
	impl From<c_int> for Any {
		fn from(val : c_int) -> Self { Any::Int(val) }
	}
	impl From<c_char> for Any {
		fn from(val : c_char) -> Self { Any::Char(val as u8 as char) }
	}
	impl From<bool> for Any {
		fn from(val : bool) -> Self { Any::Bool(val) }
	}
	impl From<c_float> for Any {
		fn from(val : c_float) -> Self { Any::Float(val) }
	}
	impl From<c_double> for Any {
		fn from(val : c_double) -> Self { Any::Double(val) }
	}

	pub fn initialize() -> Result<(), &'static str> {
		if unsafe { metacall_initialize() } != 0 {
			Err("Metacall failed to initialize")
		} else {
			Ok(())
		}
	}

	pub fn load_from_file(tag : String, scripts : Vec<String>) -> Result<(), &'static str> {
		let size : usize = scripts.len();
		// allocate a safe C String
		let ctag = CString::new(tag).expect("Conversion to C String failed");
		let mut c_scripts : Vec<*const u8> = scripts.clone().into_iter().map(|x| x.as_bytes().as_ptr()).collect();
		if unsafe { metacall_load_from_file(ctag.as_ptr(), c_scripts.as_mut_ptr(), size, std::ptr::null_mut()) } != 0 {
			return Err("MetaCall failed to load script");
		}
		Ok(())
	}

	// Possible types as variants in Rust
	pub fn metacall(func : String, args : Vec<Any>) -> Result<Any, &'static str> {
		let c_function = CString::new(func).expect("Conversion to C String failed");
		let size = args.len();
		unsafe { 
			// let c_func = metacall_function(c_function.as_ptr());
			let c_func : *mut c_void = metacall_function(c_function.as_ptr());
			if c_func.is_null() {
				return Err("Function Not Found")
			}
			let mut c_args : Vec<*mut c_void> = Vec::with_capacity(size);
			for arg in args {
				match arg {
					Any::Int(x) => {
						let a = metacall_value_create_int(x);
						c_args.push(a);
					},
					Any::Double(x) => {
						let a = metacall_value_create_double(x);
						c_args.push(a);
					},
					Any::Bool(x) => {
						let a = metacall_value_create_bool(x as c_int);
						c_args.push(a);
					},
					Any::Char(x) => {
						let a = metacall_value_create_char(x as c_char);
						c_args.push(a);
					},
					Any::Str(x) => {
						let st = CString::new(x.clone()).expect("can't convert to c str");
						let a = metacall_value_create_string(st.as_ptr(), x.len());
						c_args.push(a);
					},
					_ => {}
				}
			}
			let ret : *mut c_void = metacallfv(c_func, c_args.as_mut_ptr());
			let mut rt = Any::Null;
			if !ret.is_null() {
				match metacall_value_id(ret) {
					0 => {
						rt = Any::Bool(if metacall_value_to_bool(ret) != 0 { true } else { false });
					},
					1 => {
						//METACALL_CHAR
					},
					2 => {
						//METACALL_SHORT
					},
					3 => {
						rt = Any::Int(metacall_value_to_int(ret));
					},
					4 => {
						// METACALL_LONG
					},
					5 => {
						// METACALL_FLOAT
					},
					6 => {
						rt = Any::Double(metacall_value_to_double(ret));
					},
					7 => {
						let st = std::ffi::CStr::from_ptr(metacall_value_to_string(ret));
						rt = Any::Str(String::from(st.to_str().expect("couldn't convert CStr to &str")));
					},
					8 => {
						// METACALL_BUFFER
					},
					9 => {
						// METACALL_ARRAY
					},
					10 => {
						// METACALL_MAP
					},
					11 => {
						// METACALL_PTR
					},
					12 => {
						// METACALL_FUTURE
					},
					13 => {
						// METACALL_FUNCTION
					},
					14 => {
						// METACALL_NULL
					},
					_ => {}
				}
				metacall_value_destroy(ret);
			}
			for arg in c_args {
				metacall_value_destroy(arg);
			}
			return Ok(rt);
		}
	}

	pub fn destroy() {
		unsafe { metacall_destroy(); }
	}
}

/// Doc test to check if the code can build an run
#[test]
fn it_works() {
	let _d = defer(|| metacall::destroy());

	match metacall::initialize() {
		Err(e) => { println!("{}", e); panic!(); }, 
		_ => { println!(" Hello World Metacall created ") }
	}

	let scripts : Vec<String> = vec!["test.mock".to_string()];

	match metacall::load_from_file("mock".to_string(), scripts) {
		Err(e) => { println!("{}", e); panic!(); }, 
		_ => ()
	}

	match metacall::metacall("new_args".to_string(),
							vec![
								metacall::Any::Char('a')
								])
	{
		Err(e) => { println!("{}", e); panic!(); }, 
		Ok(ret) => { println!("{:?}", ret); }
	}
}
