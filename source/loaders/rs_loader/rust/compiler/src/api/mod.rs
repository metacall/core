use std::{
    ffi::{c_void, CString},
    os::raw::{c_char, c_int},
    path::PathBuf,
};
pub type OpaqueType = *mut c_void;
pub type OpaqueTypeList = *mut OpaqueType;
mod class;
mod function;
mod object;
pub use function::{
    function_singleton, register_function, FunctionCreate, FunctionInputSignature,
    FunctionRegistration,
};

pub use class::{class_singleton, register_class, ClassCreate, ClassRegistration};

pub struct LoaderLifecycleState {
    pub execution_paths: Vec<PathBuf>,
    pub destroy_list: Vec<super::DynlinkLibrary>,
}
impl LoaderLifecycleState {
    pub fn new(execution_paths: Vec<PathBuf>) -> LoaderLifecycleState {
        LoaderLifecycleState {
            execution_paths,
            destroy_list: vec![],
        }
    }
}
extern "C" {
    fn loader_impl_get(loader_impl: OpaqueType) -> OpaqueType;

    fn loader_initialization_register(loader_impl: OpaqueType);

    fn loader_unload_children(loader_impl: OpaqueType);

    fn loader_impl_type_define(
        loader_impl: OpaqueType,
        name: *const c_char,
        the_type: OpaqueType,
    ) -> c_int;

    fn type_create(
        type_id: c_int,
        name: *const c_char,
        type_impl: OpaqueType,
        singleton: OpaqueType,
    ) -> OpaqueType;

    fn type_name(t: OpaqueType) -> *const c_char;

    fn function_create(
        name: *const c_char,
        args_count: usize,
        function_impl: OpaqueType,
        singleton: OpaqueType,
    ) -> OpaqueType;

    fn signature_set(signature: OpaqueType, index: usize, name: *const c_char, t: OpaqueType);

    fn context_scope(ctx: OpaqueType) -> OpaqueType;

    fn function_name(function: OpaqueType) -> *mut c_char;

    fn function_signature(function: OpaqueType) -> OpaqueType;

    fn value_create_function(function: OpaqueType) -> OpaqueType;

    fn value_type_destroy(v: OpaqueType);

    fn signature_set_return(signature: OpaqueType, t: OpaqueType);

    fn loader_impl_type(loader_impl: OpaqueType, name: *const c_char) -> OpaqueType;

    fn scope_define(scope: OpaqueType, key: *mut c_char, value: OpaqueType) -> c_int;
    // REFLECT_API klass class_create(const char *name, enum accessor_type_id accessor, class_impl impl, class_impl_interface_singleton singleton);
    fn class_create(
        name: *const c_char,
        accessor_type_id: c_int,
        class_impl: OpaqueType,
        singleton: OpaqueType,
    ) -> OpaqueType;
    fn value_create_class(class: OpaqueType) -> OpaqueType;
    fn class_name(class: OpaqueType) -> *mut c_char;
    // constructor constructor_create(size_t count, enum class_visibility_id visibility);
    fn constructor_create(count: usize, visibility: c_int) -> OpaqueType;
    fn constructor_set(ctor: OpaqueType, index: usize, name: *const c_char, t: OpaqueType);
    fn class_register_constructor(class: OpaqueType, ctor: OpaqueType) -> c_int;
    fn attribute_create(
        class: OpaqueType,
        name: *const c_char,
        t: OpaqueType,
        attr_impl: OpaqueType,
        visibility: c_int,
        singleton: OpaqueType,
    ) -> OpaqueType;
    // fn class_register_static_attribute(class: OpaqueType, attr: OpaqueType) -> c_int;
    fn class_register_attribute(class: OpaqueType, attr: OpaqueType) -> c_int;
    fn get_attr_name(attr: OpaqueType) -> *mut c_char;
    fn method_create(
        class: OpaqueType,
        name: *const c_char,
        args_count: usize,
        method_impl: OpaqueType,
        visibility: c_int,
        async_id: c_int,
        singleton: OpaqueType,
    ) -> OpaqueType;
    fn class_register_static_method(class: OpaqueType, method: OpaqueType) -> c_int;
    fn class_register_method(class: OpaqueType, method: OpaqueType) -> c_int;
    fn method_signature(method: OpaqueType) -> OpaqueType;
    fn method_name(method: OpaqueType) -> *mut c_char;
    fn object_create(
        name: *const c_char,
        accessor_id: c_int,
        object_impl: OpaqueType,
        singleton: OpaqueType,
        class: OpaqueType,
    ) -> OpaqueType;

}

pub fn get_loader_lifecycle_state(loader_impl: OpaqueType) -> *mut LoaderLifecycleState {
    let loader_lifecycle_state =
        unsafe { loader_impl_get(loader_impl) } as *mut LoaderLifecycleState;

    loader_lifecycle_state
}

pub fn loader_lifecycle_register(loader_impl: OpaqueType) {
    unsafe { loader_initialization_register(loader_impl) };
}

pub fn loader_lifecycle_unload_children(loader_impl: OpaqueType) {
    unsafe { loader_unload_children(loader_impl) };
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
    loader_impl: OpaqueType,
    name: &str,
    type_id: PrimitiveMetacallProtocolTypes,
    type_impl: OpaqueType,
    singleton: OpaqueType,
) {
    let name = CString::new(name).expect("Failed to convert type name to C string");
    let type_id = type_id as c_int;

    unsafe {
        let t = type_create(type_id, name.as_ptr(), type_impl, singleton);

        loader_impl_type_define(loader_impl, type_name(t), t)
    };
}
