use std::{ffi::CString, os::raw::c_int};

use super::*;
use crate::wrapper::class;
#[repr(C)]
pub struct FunctionInterface {
    create: extern "C" fn(OpaqueType, OpaqueType) -> c_int,
    invoke: extern "C" fn(OpaqueType, OpaqueType, OpaqueTypeList, usize) -> OpaqueType,
    r#await: extern "C" fn(
        OpaqueType,
        OpaqueType,
        OpaqueTypeList,
        usize,
        extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
        extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
        OpaqueType,
    ) -> OpaqueType,
    destroy: extern "C" fn(OpaqueType, OpaqueType),
}

#[no_mangle]
extern "C" fn function_singleton_create(_func: OpaqueType, _func_impl: OpaqueType) -> c_int {
    0
}

#[no_mangle]
extern "C" fn function_singleton_invoke(
    _func: OpaqueType,
    func_impl: OpaqueType,
    args_p: OpaqueTypeList,
    size: usize,
) -> OpaqueType {
    unsafe {
        let args = std::slice::from_raw_parts(args_p, size).to_vec();
        let nf = Box::from_raw(func_impl as *mut class::NormalFunction);
        let res = nf.invoke(args).expect("Function return error");

        std::mem::forget(nf);
        res
    }
}

#[no_mangle]
extern "C" fn function_singleton_await(
    _func: OpaqueType,
    _func_impl: OpaqueType,
    _args: OpaqueTypeList,
    _size: usize,
    _resolve: extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
    _reject: extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
    _data: OpaqueType,
) -> OpaqueType {
    println!("rs_loader: await function");
    0 as OpaqueType
}

#[no_mangle]
extern "C" fn function_singleton_destroy(_func: OpaqueType, func_impl: OpaqueType) {
    if !func_impl.is_null() {
        unsafe {
            let func_ptr = Box::from_raw(func_impl as *mut class::NormalFunction);
            drop(func_ptr);
        }
    }
}

#[no_mangle]
pub extern "C" fn function_singleton() -> *const FunctionInterface {
    static SINGLETON: FunctionInterface = FunctionInterface {
        create: function_singleton_create,
        invoke: function_singleton_invoke,
        r#await: function_singleton_await,
        destroy: function_singleton_destroy,
    };

    &SINGLETON
}

pub struct FunctionCreate {
    pub name: String,
    pub args_count: usize,
    pub singleton: OpaqueType,
    pub function_impl: OpaqueType,
}
pub struct FunctionInputSignature {
    pub name: String,
    pub t: String,
}
pub struct FunctionRegistration {
    pub ctx: OpaqueType,
    pub loader_impl: OpaqueType,
    pub function_create: FunctionCreate,
    pub ret: Option<String>,
    pub input: Vec<FunctionInputSignature>,
}

pub fn register_function(function_registration: FunctionRegistration) {
    let sp = unsafe { context_scope(function_registration.ctx) };

    let FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton,
    } = function_registration.function_create;
    let name = CString::new(name).expect("Failed to convert function name to C string");
    let f = unsafe { function_create(name.as_ptr(), args_count, function_impl, singleton) };

    let s = unsafe { function_signature(f) };

    if let Some(ret) = function_registration.ret {
        let ret = CString::new(ret).expect("Failed to convert return type to C string");

        unsafe {
            signature_set_return(
                s,
                loader_impl_type(function_registration.loader_impl, ret.as_ptr()),
            );
        };
    } else {
        let ret = CString::new("Null").expect("Failed to convert return type to C string");

        unsafe {
            signature_set_return(
                s,
                loader_impl_type(function_registration.loader_impl, ret.as_ptr()),
            );
        };
    }

    for (index, param) in function_registration.input.iter().enumerate() {
        let name = CString::new(param.name.clone())
            .expect("Failed to convert function parameter name to C string");
        let t = CString::new(param.t.clone())
            .expect("Failed to convert function parameter type to C string");

        unsafe {
            signature_set(
                s,
                index,
                name.as_ptr(),
                loader_impl_type(function_registration.loader_impl, t.as_ptr()),
            )
        };
    }

    unsafe {
        let v = value_create_function(f);
        if scope_define(sp, function_name(f), v) != 0 {
            value_type_destroy(v);
            // TODO: Should return error
        }
    };
}
