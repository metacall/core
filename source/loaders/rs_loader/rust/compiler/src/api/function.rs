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
extern "C" fn function_rs_interface_create(_func: OpaqueType, _func_impl: OpaqueType) -> c_int {
    0
}

#[no_mangle]
extern "C" fn function_rs_interface_invoke(
    _func: OpaqueType,
    func_impl: OpaqueType,
    args_p: OpaqueTypeList,
    size: usize,
) -> OpaqueType {
    unsafe {
        let args = std::slice::from_raw_parts(args_p, size).to_vec();
        let nf = Box::from_raw(func_impl as *mut class::Function);
        let res = nf.invoke(args).expect("Function return error");

        std::mem::forget(nf);
        res
    }
}

#[no_mangle]
extern "C" fn function_rs_interface_await(
    _func: OpaqueType,
    _func_impl: OpaqueType,
    _args: OpaqueTypeList,
    _size: usize,
    _resolve: extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
    _reject: extern "C" fn(OpaqueType, OpaqueType) -> OpaqueType,
    _data: OpaqueType,
) -> OpaqueType {
    eprintln!("Rust Loader: Function await not implemented");
    0 as OpaqueType
}

#[no_mangle]
extern "C" fn function_rs_interface_destroy(_func: OpaqueType, func_impl: OpaqueType) {
    if !rs_loader_destroyed() && !func_impl.is_null() {
        unsafe {
            let func_ptr = Box::from_raw(func_impl as *mut class::Function);
            drop(func_ptr);
        }
    }
}

#[no_mangle]
pub extern "C" fn function_rs_singleton() -> *const FunctionInterface {
    static SINGLETON: FunctionInterface = FunctionInterface {
        create: function_rs_interface_create,
        invoke: function_rs_interface_invoke,
        r#await: function_rs_interface_await,
        destroy: function_rs_interface_destroy,
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
    pub template: Option<OpaqueType>,
}

/// Creates a MetaCall function and sets its signature from the Rust types
/// of its arguments and return value.
///
/// # Safety
/// `loader_impl` must be a valid MetaCall loader implementation and the
/// implementation and singleton of `create` must be valid MetaCall pointers.
pub unsafe fn create_function_with_signature(
    loader_impl: OpaqueType,
    create: FunctionCreate,
    ret: Option<String>,
    input: &[FunctionInputSignature],
) -> OpaqueType {
    let FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton,
    } = create;
    let name = CString::new(name).expect("Failed to convert function name to C string");
    let f = unsafe { function_create(name.as_ptr(), args_count, function_impl, singleton) };

    if f.is_null() {
        return f;
    }

    let s = unsafe { function_signature(f) };

    let ret = CString::new(ret.unwrap_or_else(|| "Null".to_string()))
        .expect("Failed to convert return type to C string");

    unsafe {
        signature_set_return(s, loader_impl_type(loader_impl, ret.as_ptr()));
    };

    for (index, param) in input.iter().enumerate() {
        let name = CString::new(param.name.clone())
            .expect("Failed to convert function parameter name to C string");
        let t = CString::new(param.t.clone())
            .expect("Failed to convert function parameter type to C string");

        unsafe {
            signature_set(
                s,
                index,
                name.as_ptr(),
                loader_impl_type(loader_impl, t.as_ptr()),
            )
        };
    }

    f
}

pub fn register_function(function_registration: FunctionRegistration) {
    unsafe {
        let f = create_function_with_signature(
            function_registration.loader_impl,
            function_registration.function_create,
            function_registration.ret,
            &function_registration.input,
        );

        register_existing_function(function_registration.ctx, f);
    }
}

/// # Safety
/// `ctx` must be a valid MetaCall context and `function` must be a valid
/// MetaCall function.
pub unsafe fn register_existing_function(ctx: OpaqueType, function: OpaqueType) {
    let sp = context_scope(ctx);
    let name = function_name(function);

    let v = value_create_function(function);

    let result = scope_define(sp, name, v);

    if result != 0 {
        value_type_destroy(v);
    }
}
