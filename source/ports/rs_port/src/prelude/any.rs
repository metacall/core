use super::{MetacallClass, MetacallException, MetacallFunction, MetacallFuture, MetacallObject};
use crate::{
    bindings::*,
    cstring,
    parsers::{self, any_to_metacall},
};
use std::{
    collections::HashMap,
    ffi::{c_char, c_int, c_void, CStr},
    fmt::{Debug, Formatter},
    hash::{Hash, Hasher},
    mem, slice,
};

/// Enum of all possible Metacall types to allow for safe conversion between them and c_types
#[derive(Clone)]
pub enum Any {
    Array(Vec<Box<Any>>),
    Boolean(bool),
    Buffer(Vec<u8>),
    Char(char),
    Class(MetacallClass),
    Double(f64),
    Exception(MetacallException),
    Float(f32),
    Function(MetacallFunction),
    Future(MetacallFuture),
    Int(i32),
    Long(i64),
    Map(HashMap<Box<Any>, Box<Any>>),
    Null,
    Object(MetacallObject),
    Pointer(Box<Any>),
    Short(i16),
    String(String),
    Throwable(MetacallException),
}

impl Debug for Any {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), std::fmt::Error> {
        match self {
            Any::Array(a) => write!(f, "Any::Array({:#?})", a),
            Any::Boolean(a) => write!(f, "Any::Bool({:#?})", a),
            Any::Buffer(a) => write!(f, "Any::Buffer({:#?})", a),
            Any::Char(a) => write!(f, "Any::Char({:#?})", a),
            Any::Class(_) => write!(f, "Any::Class"),
            Any::Double(a) => write!(f, "Any::Double({:#?})", a),
            Any::Exception(_) => write!(f, "Any::Exception"),
            Any::Float(a) => write!(f, "Any::Float({:#?})", a),
            Any::Function(_) => write!(f, "Any::Function"),
            Any::Future(_) => write!(f, "Any::Future"),
            Any::Int(a) => write!(f, "Any::Int({:#?})", a),
            Any::Long(a) => write!(f, "Any::Long({:#?})", a),
            Any::Map(a) => write!(f, "Any::Map({:#?})", a),
            Any::Null => write!(f, "Any::Null"),
            Any::Object(_) => write!(f, "Any::Object"),
            Any::Pointer(a) => write!(f, "Any::Pointer({:#?})", a),
            Any::Short(a) => write!(f, "Any::Short({:#?})", a),
            Any::String(a) => write!(f, "Any::Str({:#?})", a),
            Any::Throwable(_) => write!(f, "Any::Throwable"),
        }
    }
}
impl Eq for Any {}
impl Hash for Any {
    fn hash<H>(&self, hasher: &mut H)
    where
        H: Hasher,
    {
        match self {
            Any::Array(a) => a.hash(hasher),
            Any::Boolean(a) => a.hash(hasher),
            Any::Buffer(a) => a.hash(hasher),
            Any::Char(a) => a.hash(hasher),
            Any::Class(a) => a.hash(hasher),
            Any::Double(a) => a.to_string().hash(hasher),
            Any::Exception(a) => a.hash(hasher),
            Any::Float(a) => a.to_string().hash(hasher),
            Any::Function(a) => a.hash(hasher),
            Any::Future(a) => a.hash(hasher),
            Any::Int(a) => a.hash(hasher),
            Any::Long(a) => a.hash(hasher),
            Any::Map(_) => None::<Option<()>>.hash(hasher),
            Any::Null => None::<Option<()>>.hash(hasher),
            Any::Object(a) => a.hash(hasher),
            Any::Pointer(a) => a.hash(hasher),
            Any::Short(a) => a.hash(hasher),
            Any::String(a) => a.hash(hasher),
            Any::Throwable(a) => a.hash(hasher),
        }
    }
}

impl PartialEq for Any {
    fn eq(&self, other: &Self) -> bool {
        self == other
    }
}

impl Any {
    pub fn from_boolean(v: *mut c_void) -> Any {
        Any::Boolean(unsafe { metacall_value_to_bool(v) } != 0)
    }
    pub fn from_char(v: *mut c_void) -> Any {
        Any::Char(unsafe { metacall_value_to_char(v) } as u8 as char)
    }
    pub fn from_short(v: *mut c_void) -> Any {
        Any::Short(unsafe { metacall_value_to_short(v) })
    }
    pub fn from_int(v: *mut c_void) -> Any {
        Any::Int(unsafe { metacall_value_to_int(v) })
    }
    pub fn from_long(v: *mut c_void) -> Any {
        Any::Long(unsafe { metacall_value_to_long(v) })
    }
    pub fn from_float(v: *mut c_void) -> Any {
        Any::Float(unsafe { metacall_value_to_float(v) })
    }
    pub fn from_double(v: *mut c_void) -> Any {
        Any::Double(unsafe { metacall_value_to_double(v) })
    }
    pub fn from_string(v: *mut c_void) -> Any {
        let c_str = unsafe { CStr::from_ptr(metacall_value_to_string(v)) };

        Any::String(String::from(c_str.to_str().unwrap()))
    }
    pub fn from_buffer(v: *mut c_void) -> Any {
        Any::Buffer(unsafe {
            let arr = metacall_value_to_buffer(v);
            let count = metacall_value_count(v);

            let slice = slice::from_raw_parts(arr, count as usize)
                .iter()
                .map(|p| *mem::transmute::<&c_void, Box<u8>>(p))
                .collect::<Vec<u8>>();

            slice
        })
    }
    pub fn from_array(v: *mut c_void) -> Any {
        Any::Array(unsafe {
            let arr = metacall_value_to_array(v);
            let count = metacall_value_count(v);

            let mut new_arr = Vec::with_capacity(count as usize);
            slice::from_raw_parts(arr, count as usize)
                .iter()
                .for_each(|p| {
                    new_arr.push(Box::new(parsers::metacall_to_any(*p)));
                });

            new_arr
        })
    }
    pub fn from_map(v: *mut c_void) -> Any {
        Any::Map(unsafe {
            let map = metacall_value_to_map(v);
            let count = metacall_value_count(v);
            let map = std::slice::from_raw_parts(map, count as usize);
            let mut r_map: HashMap<Box<Any>, Box<Any>> = HashMap::new();
            for map_value in map {
                let m_pair = metacall_value_to_array(*map_value);
                let m_pair = std::slice::from_raw_parts(m_pair, 2);

                let key = parsers::metacall_to_any(m_pair[0]);
                let val = parsers::metacall_to_any(m_pair[1]);

                r_map.insert(Box::new(key), Box::new(val));
            }

            r_map
        })
    }
    pub fn from_pointer(v: *mut c_void) -> Any {
        let ptr = unsafe { metacall_value_to_ptr(v) };
        let ptr_type = parsers::metacall_to_any(ptr);

        Any::Pointer(Box::new(ptr_type))
    }
    pub fn from_future(v: *mut c_void) -> Any {
        let future = unsafe { metacall_value_to_future(v) };

        Any::Future(MetacallFuture::new(future))
    }
    pub fn from_function(v: *mut c_void) -> Any {
        let function = unsafe { metacall_value_to_function(v) };

        Any::Function(MetacallFunction::new_raw(function))
    }
    pub fn from_null(v: *mut c_void) -> Any {
        let _null = unsafe { metacall_value_to_null(v) };

        Any::Null
    }
    pub fn from_class(v: *mut c_void) -> Any {
        let class = unsafe { metacall_value_to_class(v) };

        Any::Class(MetacallClass::from(class))
    }
    pub fn from_object(v: *mut c_void) -> Any {
        let obj = unsafe { metacall_value_to_object(v) };

        Any::Object(MetacallObject::new(obj))
    }
    pub fn from_exception(v: *mut c_void) -> Any {
        let exception = unsafe { metacall_value_to_exception(v) };

        Any::Exception(MetacallException::new(exception, false))
    }
    pub fn from_throwable(v: *mut c_void) -> Any {
        let throwable = unsafe { metacall_value_to_throwable(v) };

        Any::Throwable(MetacallException::new(throwable, true))
    }
}

impl Any {
    pub fn to_boolean(v: bool) -> *mut c_void {
        unsafe { metacall_value_create_bool((v as c_int).try_into().unwrap()) }
    }
    pub fn to_char(v: char) -> *mut c_void {
        unsafe { metacall_value_create_char(v as c_char) }
    }
    pub fn to_short(v: i16) -> *mut c_void {
        unsafe { metacall_value_create_short(v) }
    }
    pub fn to_int(v: i32) -> *mut c_void {
        unsafe { metacall_value_create_int(v) }
    }
    pub fn to_long(v: i64) -> *mut c_void {
        unsafe { metacall_value_create_long(v) }
    }
    pub fn to_float(v: f32) -> *mut c_void {
        unsafe { metacall_value_create_float(v) }
    }
    pub fn to_double(v: f64) -> *mut c_void {
        unsafe { metacall_value_create_double(v) }
    }
    pub fn to_str(v: String) -> *mut c_void {
        let c_str = cstring!(v.as_str()).unwrap();

        unsafe { metacall_value_create_string(c_str.as_ptr(), v.len()) }
    }
    pub fn to_buffer(mut v: Vec<u8>) -> *mut c_void {
        unsafe { metacall_value_create_buffer(v.as_mut_ptr() as *mut c_void, v.len()) }
    }
    pub fn to_array(v: Vec<Box<Any>>) -> *mut c_void {
        let mut new_arr: Vec<*const c_void> = Vec::new();

        for ar in v {
            new_arr.push(parsers::any_to_metacall([*ar])[0]);
        }

        unsafe { metacall_value_create_array(new_arr.as_mut_ptr(), new_arr.len()) }
    }
    pub fn to_map(v: HashMap<Box<Any>, Box<Any>>) -> *mut c_void {
        let mut map: Vec<*const c_void> = v
            .into_iter()
            .map(|v| {
                Box::into_raw(Box::new((
                    any_to_metacall([*v.0])[0] as *const c_void,
                    any_to_metacall([*v.1])[0] as *const c_void,
                ))) as *const c_void
            })
            .collect();

        unsafe { metacall_value_create_map(map.as_mut_ptr(), map.len()) }
    }
    pub fn to_pointer(v: Box<Any>) -> *mut c_void {
        let ptr = parsers::any_to_metacall([*v])[0];

        unsafe { metacall_value_create_ptr(ptr) }
    }
    pub fn to_future(v: MetacallFuture) -> *mut c_void {
        v.into_raw()
    }
    pub fn to_function(v: MetacallFunction) -> *mut c_void {
        v.into_raw()
    }
    pub fn to_null() -> *mut c_void {
        unsafe { metacall_value_create_null() }
    }
    pub fn to_class(v: MetacallClass) -> *mut c_void {
        v.into_raw()
    }
    pub fn to_object(v: MetacallObject) -> *mut c_void {
        v.into_raw()
    }
    pub fn to_exception(v: MetacallException) -> *mut c_void {
        v.into_raw()
    }
    pub fn to_throwable(v: MetacallException) -> *mut c_void {
        v.into_raw()
    }
}
