use std::{ffi::c_void, os::raw::c_uint};

use api::{register_function, function_singleton, FunctionCreate, FunctionInputSignature, FunctionRegisteration};

use crate::file::DlopenLibrary;
use libffi::low::CodePtr;

use crate::{CompilerState, Function};

fn function_create(func: &Function, dlopen_library: &DlopenLibrary) -> FunctionCreate {
    let name = func.name.clone();
    let args_count = func.args.len();

    let function_ptr: unsafe fn() = unsafe { dlopen_library.instance.symbol(&name[..]) }.unwrap();

    let libffi_func = Box::new(CodePtr::from_ptr(function_ptr as *const c_void));

    let function_impl = Box::into_raw(libffi_func) as *mut c_void;

    let function_create = FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton: function_singleton as *mut c_void// 0 as c_uint as *mut c_void, // TODO: This must be a function pointer to 'function_singleton' inside the API module
    };

    function_create
}

pub fn register(state: &CompilerState, dlopen_library: &DlopenLibrary, loader_impl: *mut c_void, ctx: *mut c_void) {
    for func in state.functions.iter() {
        let function_registration = FunctionRegisteration {
            ctx,
            loader_impl,
            function_create: function_create(func, &dlopen_library),
            ret: match &func.ret {
                Some(ret) => Some(ret.name.clone()),
                _ => None,
            },
            input: func
                .args
                .iter()
                .map(|param| FunctionInputSignature {
                    name: param.name.clone(),
                    t: param.t.name.clone(),
                })
                .collect(),
        };

        register_function(function_registration);
    }
}
