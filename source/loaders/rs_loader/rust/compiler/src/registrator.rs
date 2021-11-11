use std::{ffi::c_void, os::raw::c_uint};

use api::{
    register_function, FunctionCreate, FunctionInputSignature, FunctionRegisteration,
};

use crate::{CompilerState};


pub fn register(state: &CompilerState, loader_impl: *mut c_void, ctx: *mut c_void) {
    for func in state.functions.iter() {
        let function_create = FunctionCreate {
            name: func.name.clone(),
            args_count: func.args.len(),
            function_impl: 0 as c_uint as *mut c_void, // TODO: Store the function pointer address here from DlLibrary
            singleton: 0 as c_uint as *mut c_void, // TODO: Create the singleton for handling the calls
        };

        let function_registration = FunctionRegisteration {
            ctx,
            loader_impl,
            function_create,
            ret: match &func.ret {
                Some(ret) => Some(ret.name.clone()),
                _ => None,
            },
            input: func.args.iter().map(|param| {
                FunctionInputSignature {
                    name: param.name.clone(),
                    t: param.t.name.clone(),
                }
            }).collect(),
        };

        register_function(function_registration);
    }
}
