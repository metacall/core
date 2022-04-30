use std::ffi::c_void;

use api::{
    function_singleton, register_function, FunctionCreate, FunctionInputSignature,
    FunctionRegisteration,
};

// use dlopen::raw::Library as DlopenLibrary;
use crate::{CompilerState, DlopenLibrary, Function};

fn function_create(func: &Function, dlopen_library: &DlopenLibrary) -> FunctionCreate {
    let name = func.name.clone();
    let args_count = func.args.len();

    let function_ptr: unsafe fn() = unsafe { dlopen_library.instance.symbol(&name[..]) }.unwrap();
    let function_impl = Box::into_raw(Box::new(function_ptr)) as *mut c_void;

    let function_create = FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton: function_singleton as *mut c_void,
    };

    function_create
}

pub fn register(
    state: &CompilerState,
    dlopen_library: &DlopenLibrary,
    loader_impl: *mut c_void,
    ctx: *mut c_void,
) {
    for func in state.functions.iter() {
        let function_registration = FunctionRegisteration {
            ctx,
            loader_impl,
            function_create: function_create(func, &dlopen_library),
            ret: match &func.ret {
                Some(ret) => Some(ret.ty.to_string().clone()),
                _ => None,
            },
            input: func
                .args
                .iter()
                .map(|param| FunctionInputSignature {
                    name: param.name.clone(),
                    t: param.ty.to_string().clone(),
                })
                .collect(),
        };

        register_function(function_registration);
    }
}
