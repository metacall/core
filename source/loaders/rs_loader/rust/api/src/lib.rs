use std::{
    ffi::{c_void, CString},
    fmt::Display,
    os::raw::{c_char, c_int},
    path::PathBuf,
};

pub struct LoaderLifecycleState {
    pub execution_paths: Vec<PathBuf>,
}
impl LoaderLifecycleState {
    pub fn new(execution_paths: Vec<PathBuf>) -> LoaderLifecycleState {
        LoaderLifecycleState { execution_paths }
    }
}

fn construct_c_char<T: Display>(str: T) -> *mut i8 {
    CString::new(format!("{}", str))
        .unwrap()
        .as_bytes()
        .as_ptr() as *mut i8
}

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;

    fn loader_impl_type_define(
        loader_impl: *mut c_void,
        name: *mut c_char,
        the_type: *mut c_void,
    ) -> c_int;

    fn type_create(
        type_id: c_int,
        name: *mut c_char,
        type_impl: *mut c_void,
        singleton: *mut c_void,
    ) -> *mut c_void;

    fn function_create(
        name: *mut c_char,
        args_count: usize,
        function_impl: *mut c_void,
        singleton: *mut c_void,
    ) -> *mut c_void;

    fn signature_set(
        signature: *mut c_void,
        index: usize,
        name: *mut c_char,
        t: *mut c_void,
    ) -> c_void;

    fn context_scope(ctx: *mut c_void) -> *mut c_void;

    fn function_name(function: *mut c_void) -> *mut c_char;

    fn function_signature(function: *mut c_void) -> *mut c_void;

    fn value_create_function(function: *mut c_void) -> *mut c_void;

    fn signature_set_return(signature: *mut c_void, t: *mut c_void) -> c_void;

    fn loader_impl_type(loader_impl: *mut c_void, name: *mut c_char) -> *mut c_void;

    fn scope_define(scope: *mut c_void, key: *mut c_char, value: *mut c_void) -> c_int;
}

pub fn get_loader_lifecycle_state(loader_impl: *mut c_void) -> *mut LoaderLifecycleState {
    let loader_lifecycle_state =
        unsafe { loader_impl_get(loader_impl) } as *mut LoaderLifecycleState;

    loader_lifecycle_state
}

pub enum PrimitiveMetacallProtocolTypes {
    Bool = 0,
    Char = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    Double = 6,
    String = 7,
    Buffer = 8,
    Array = 9,
    Map = 10,
    Pointer = 11,
    Future = 12,
    Function = 13,
    Null = 14,
    Class = 15,
    Object = 16,
}

pub fn define_type(
    loader_impl: *mut c_void,
    name: &str,
    type_id: PrimitiveMetacallProtocolTypes,
    type_impl: *mut c_void,
    singleton: *mut c_void,
) {
    let type_name = construct_c_char(name);
    let type_id = type_id as c_int;

    unsafe {
        loader_impl_type_define(
            loader_impl,
            type_name,
            type_create(type_id, type_name, type_impl, singleton),
        )
    };
}

pub struct FunctionCreate {
    pub name: String,
    pub args_count: usize,
    pub singleton: *mut c_void,
    pub function_impl: *mut c_void,
}
pub struct FunctionInputSignature {
    pub name: String,
    pub t: String,
}
pub struct FunctionRegisteration {
    pub ctx: *mut c_void,
    pub loader_impl: *mut c_void,
    pub function_create: FunctionCreate,
    pub ret: Option<String>,
    pub input: Vec<FunctionInputSignature>,
}

pub fn register_function(function_registeration: FunctionRegisteration) {
    let sp = unsafe { context_scope(function_registeration.ctx) };

    let FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton,
    } = function_registeration.function_create;
    let name = construct_c_char(name);
    let f = unsafe { function_create(name, args_count, function_impl, singleton) };

    let s = unsafe { function_signature(f) };

    if let Some(ret) = function_registeration.ret {
        unsafe {
            signature_set_return(
                s,
                loader_impl_type(
                    function_registeration.loader_impl,
                    construct_c_char(ret.to_string()),
                ),
            );
        };
    }

    for (index, function_input) in function_registeration
        .input
        .iter()
        .enumerate()
    {
        unsafe {
            signature_set(
                s,
                index,
                construct_c_char(function_input.name.to_owned()),
                loader_impl_type(
                    function_registeration.loader_impl,
                    construct_c_char(function_input.t.to_string()),
                ),
            )
        };
    }

    unsafe { scope_define(sp, function_name(f), value_create_function(f)) };
}
