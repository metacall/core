use crate::wrapper::class::{Class, Instance};

use super::*;
use std::{ffi::CStr, os::raw::c_int};
#[repr(C)]
pub struct ObjectInterface {
    create: extern "C" fn(OpaqueType, OpaqueType) -> c_int,
    get: extern "C" fn(OpaqueType, OpaqueType, OpaqueType) -> OpaqueType,
    set: extern "C" fn(OpaqueType, OpaqueType, OpaqueType, OpaqueType) -> c_int,
    method_invoke:
        extern "C" fn(OpaqueType, OpaqueType, OpaqueType, OpaqueTypeList, usize) -> OpaqueType,
    method_await:
        extern "C" fn(OpaqueType, OpaqueType, OpaqueType, OpaqueTypeList, usize) -> OpaqueType,
    destructor: extern "C" fn(OpaqueType, OpaqueType) -> c_int,
    destroy: extern "C" fn(OpaqueType, OpaqueType),
}

#[repr(C)]
pub struct Object {
    pub instance: Instance,
    pub class: *mut Class,
}

#[no_mangle]
extern "C" fn object_singleton_create(_object: OpaqueType, _object_impl: OpaqueType) -> c_int {
    println!("object create");
    0
}

#[no_mangle]
extern "C" fn object_singleton_set(
    _object: OpaqueType,
    object_impl: OpaqueType,
    accessor: OpaqueType,
    value: OpaqueType,
) -> c_int {
    unsafe {
        let object_impl_ptr = object_impl as *mut object::Object;
        let mut obj = Box::from_raw(object_impl_ptr);
        let class = Box::from_raw(obj.class);
        let name = CStr::from_ptr(get_attr_name(accessor))
            .to_str()
            .expect("Unable to get attr name");
        println!("object set attr: {}", name);
        obj.instance.set_attr(name, value, &class);

        std::mem::forget(class);
        std::mem::forget(obj);
        std::mem::forget(name);
    };
    0
}

#[no_mangle]
extern "C" fn object_singleton_get(
    _object: OpaqueType,
    object_impl: OpaqueType,
    accessor: OpaqueType,
) -> OpaqueType {
    let ret = unsafe {
        let object_impl_ptr = object_impl as *mut object::Object;
        let obj = Box::from_raw(object_impl_ptr);
        let class = Box::from_raw(obj.class);
        let name = CStr::from_ptr(get_attr_name(accessor))
            .to_str()
            .expect("Unable to get attr name");
        println!("object get attr: {}", name);
        let ret = obj.instance.get_attr(name, &class);

        std::mem::forget(class);
        std::mem::forget(obj);
        std::mem::forget(name);
        ret
    };
    if let Ok(ret) = ret {
        return ret;
    } else {
        return 0 as OpaqueType;
    }
}

#[no_mangle]
extern "C" fn object_singleton_method_invoke(
    _object: OpaqueType,
    object_impl: OpaqueType,
    method: OpaqueType,
    args_p: OpaqueTypeList,
    size: usize,
) -> OpaqueType {
    let ret = unsafe {
        let object_impl_ptr = object_impl as *mut object::Object;
        let obj = Box::from_raw(object_impl_ptr);
        let class = Box::from_raw(obj.class);
        let args = std::slice::from_raw_parts(args_p, size).to_vec();
        let name = CStr::from_ptr(method_name(method))
            .to_str()
            .expect("Unable to get method name");
        println!("object invoke: {}", name);
        let ret = obj.instance.call(name, args, &class);

        std::mem::forget(class);
        std::mem::forget(obj);
        std::mem::forget(name);
        ret
    };
    if let Ok(ret) = ret {
        return ret;
    } else {
        return 0 as OpaqueType;
    }
}

#[no_mangle]
extern "C" fn object_singleton_method_await(
    _object: OpaqueType,
    _object_impl: OpaqueType,
    _method: OpaqueType,
    _args_p: OpaqueTypeList,
    _size: usize,
) -> OpaqueType {
    println!("object await");
    0 as OpaqueType
}
#[no_mangle]
extern "C" fn object_singleton_destructor(_object: OpaqueType, object_impl: OpaqueType) -> c_int {
    // unsafe {
    //     let object_impl_ptr = object_impl as *mut Object;
    //     let object = Box::from_raw(object_impl_ptr);
    //     drop(object);
    // }
    println!("destruct object");
    0
}
#[no_mangle]
extern "C" fn object_singleton_destroy(_object: OpaqueType, _object_impl: OpaqueType) {
    println!("destroy object");
}

#[no_mangle]
pub extern "C" fn object_singleton() -> *const ObjectInterface {
    static SINGLETON: ObjectInterface = ObjectInterface {
        create: object_singleton_create,
        get: object_singleton_get,
        set: object_singleton_set,
        method_invoke: object_singleton_method_invoke,
        method_await: object_singleton_method_await,
        destructor: object_singleton_destructor,
        destroy: object_singleton_destroy,
    };

    &SINGLETON
}
