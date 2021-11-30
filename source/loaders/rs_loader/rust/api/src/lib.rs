use std::{
    ffi::{c_void, CString},
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

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;

    fn loader_impl_type_define(
        loader_impl: *mut c_void,
        name: *const c_char,
        the_type: *mut c_void,
    ) -> c_int;

    fn type_create(
        type_id: c_int,
        name: *const c_char,
        type_impl: *mut c_void,
        singleton: *mut c_void,
    ) -> *mut c_void;

    fn type_name(t: *mut c_void) -> *const c_char;

    fn function_create(
        name: *const c_char,
        args_count: usize,
        function_impl: *mut c_void,
        singleton: *mut c_void,
    ) -> *mut c_void;

    fn signature_set(
        signature: *mut c_void,
        index: usize,
        name: *const c_char,
        t: *mut c_void,
    );

    fn context_scope(ctx: *mut c_void) -> *mut c_void;

    fn function_name(function: *mut c_void) -> *mut c_char;

    fn function_signature(function: *mut c_void) -> *mut c_void;

    fn value_create_function(function: *mut c_void) -> *mut c_void;

    fn signature_set_return(signature: *mut c_void, t: *mut c_void);

    fn loader_impl_type(loader_impl: *mut c_void, name: *const c_char) -> *mut c_void;

    fn scope_define(scope: *mut c_void, key: *mut c_char, value: *mut c_void) -> c_int;
}

#[repr(C)]
struct function_interface {
    create: extern "C" fn(*mut c_void, *mut c_void) -> c_int,
    invoke: extern "C" fn(*mut c_void, *mut c_void, *mut *mut c_void, usize) -> *mut c_void,
    r#await: extern "C" fn(*mut c_void, *mut c_void, *mut *mut c_void, usize, extern "C" fn(*mut c_void, *mut c_void) -> *mut c_void, extern "C" fn(*mut c_void, *mut c_void) -> *mut c_void, *mut c_void) -> *mut c_void,
    destroy: extern "C" fn(*mut c_void, *mut c_void),
}

#[no_mangle]
extern "C" fn function_singleton_create(func: *mut c_void, func_impl: *mut c_void) -> c_int {
    0
}

#[no_mangle]
extern "C" fn function_singleton_invoke(func: *mut c_void, func_impl: *mut c_void, args: *mut *mut c_void, size: usize) -> *mut c_void {
    // func is of type function found here: https://github.com/metacall/core/blob/44564a0a183a121eec4a55bcb433d52a308e5e9d/source/reflect/include/reflect/reflect_function.h#L65
    // func_impl is of type: https://github.com/metacall/core/blob/44564a0a183a121eec4a55bcb433d52a308e5e9d/source/loaders/rs_loader/rust/compiler/src/registrator.rs#L19
    // args is an array of 'value' of size 'size', you can iterate over it and get the C value representation
    // The task to do is very similar to this: https://github.com/metacall/core/blob/44564a0a183a121eec4a55bcb433d52a308e5e9d/source/loaders/c_loader/source/c_loader_impl.cpp#L378
    // But implemented in Rust. We can forget about closures for now, because that's designed in order to implement callbacks

    // In the example of C invoke, the func_impl is not just the address but a struct which contains this:
    // https://github.com/metacall/core/blob/44564a0a183a121eec4a55bcb433d52a308e5e9d/source/loaders/c_loader/source/c_loader_impl.cpp#L69

    // Last element is the address, but the rest are for interfacing with the libffi API. I allocate them while discovering in order
    // to precompute the call and waste less memory and allocations during the invoke.

    0 as *mut c_void
}

#[no_mangle]
extern "C" fn function_singleton_await(func: *mut c_void, func_impl: *mut c_void, args: *mut *mut c_void, size: usize, resolve: extern "C" fn(*mut c_void, *mut c_void) -> *mut c_void, reject: extern "C" fn(*mut c_void, *mut c_void) -> *mut c_void, data: *mut c_void) -> *mut c_void {
    0 as *mut c_void
}

#[no_mangle]
extern "C" fn function_singleton_destroy(func: *mut c_void, func_impl: *mut c_void) {
    // Here we have to free the memory of this: https://github.com/metacall/core/blob/44564a0a183a121eec4a55bcb433d52a308e5e9d/source/loaders/rs_loader/rust/compiler/src/registrator.rs#L19
}

#[no_mangle]
extern "C" fn function_singleton() -> *const function_interface {
    static SINGLETON: function_interface = function_interface {
        create: function_singleton_create,
        invoke: function_singleton_invoke,
        r#await: function_singleton_await,
        destroy: function_singleton_destroy,
    };

    &SINGLETON
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
    let name = CString::new(name).expect("Failed to convert type name to C string");
    let type_id = type_id as c_int;

    unsafe {
        let t = type_create(type_id, name.as_ptr(), type_impl, singleton);

        loader_impl_type_define(loader_impl, type_name(t), t)
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
    let name = CString::new(name).expect("Failed to convert function name to C string");
    let f = unsafe { function_create(name.as_ptr(), args_count, function_impl, singleton) };

    let s = unsafe { function_signature(f) };

    if let Some(ret) = function_registeration.ret {
        let ret = CString::new(ret).expect("Failed to convert return type to C string");

        unsafe {
            signature_set_return(
                s,
                loader_impl_type(function_registeration.loader_impl, ret.as_ptr()),
            );
        };
    }

    for (index, param) in function_registeration.input.iter().enumerate() {
        let name = CString::new(param.name.clone())
            .expect("Failed to convert function parameter name to C string");
        let t = CString::new(param.t.clone())
            .expect("Failed to convert function parameter type to C string");

        unsafe {
            signature_set(
                s,
                index,
                name.as_ptr(),
                loader_impl_type(function_registeration.loader_impl, t.as_ptr()),
            )
        };
    }

    unsafe { scope_define(sp, function_name(f), value_create_function(f)) };
}
