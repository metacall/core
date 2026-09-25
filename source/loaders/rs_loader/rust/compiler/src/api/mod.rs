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

#[repr(C)]
pub struct TemplateArgument {
    pub name: *const c_char,
    pub kind: c_int,
    pub value: OpaqueType,
}

pub const TEMPLATE_ARGUMENT_TYPE: c_int = 0;
pub const TEMPLATE_ARGUMENT_TEMPLATE: c_int = 1;

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

    fn value_create_int(i: c_int) -> OpaqueType;

    fn function_call(func: OpaqueType, args: OpaqueTypeList, size: usize) -> OpaqueType;

    fn function_signature(function: OpaqueType) -> OpaqueType;

    fn value_create_function(function: OpaqueType) -> OpaqueType;

    fn value_type_destroy(v: OpaqueType);

    fn signature_set_return(signature: OpaqueType, t: OpaqueType);

    fn loader_impl_type(loader_impl: OpaqueType, name: *const c_char) -> OpaqueType;

    fn scope_define(scope: OpaqueType, key: *mut c_char, value: OpaqueType) -> c_int;
    fn class_create(
        name: *const c_char,
        accessor_type_id: c_int,
        class_impl: OpaqueType,
        singleton: OpaqueType,
    ) -> OpaqueType;
    fn value_create_class(class: OpaqueType) -> OpaqueType;
    fn class_name(class: OpaqueType) -> *mut c_char;
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
    fn value_to_int(v: OpaqueType) -> i32;

    fn metacall_loader(tag: *const c_char) -> OpaqueType;
    fn loader_is_destroyed(loader_impl: OpaqueType) -> i32;

    fn template_create(name: *const c_char, template_type: c_int) -> OpaqueType;

    fn template_add_parameter(tpl: OpaqueType, parameter: *const c_char) -> c_int;

    fn template_name(tpl: OpaqueType) -> *const c_char;

    fn template_type(tpl: OpaqueType) -> c_int;

    fn template_parameter_count(tpl: OpaqueType) -> usize;

    fn template_parameter(tpl: OpaqueType, index: usize) -> *const c_char;

    fn template_destroy(tpl: OpaqueType) -> ();

    fn template_instantiate_function(tpl: OpaqueType, args: OpaqueType, size: usize) -> OpaqueType;
}

pub const TEMPLATE_TYPE_FUNCTION: c_int = 0;
pub const TEMPLATE_TYPE_CLASS: c_int = 1;

/// Creates a MetaCall function from an implementation and interface.
///
/// # Safety
/// `function_impl` and `singleton` must be valid MetaCall pointers.
pub unsafe fn create_function(
    name: &str,
    args_count: usize,
    function_impl: OpaqueType,
    singleton: OpaqueType,
) -> OpaqueType {
    let name = CString::new(name).expect("function name contains NUL");

    unsafe { function_create(name.as_ptr(), args_count, function_impl, singleton) }
}

/// # Safety
/// `name` must be a valid MetaCall template name.
pub unsafe fn create_template(name: &str, template_type: c_int) -> OpaqueType {
    let name = CString::new(name).expect("template name contains NUL");

    unsafe { template_create(name.as_ptr(), template_type) }
}

/// # Safety
/// `tpl` must be a valid reflect_template.
pub unsafe fn add_template_parameter(tpl: OpaqueType, parameter: &str) -> c_int {
    let parameter = CString::new(parameter).expect("template parameter contains NUL");

    unsafe { template_add_parameter(tpl, parameter.as_ptr()) }
}

/// # Safety
/// `tpl` must be a valid `reflect_template`.
pub unsafe fn get_template_name(tpl: OpaqueType) -> *const c_char {
    unsafe { template_name(tpl) }
}

/// # Safety
/// `tpl` must be a valid `reflect_template`.
pub unsafe fn get_template_type(tpl: OpaqueType) -> c_int {
    unsafe { template_type(tpl) }
}

/// # Safety
/// `tpl` must be a valid `reflect_template`.
pub unsafe fn get_template_parameter_count(tpl: OpaqueType) -> usize {
    unsafe { template_parameter_count(tpl) }
}

/// # Safety
/// `tpl` must be a valid `reflect_template` and `index` must be within
/// the template's parameter range.
pub unsafe fn get_template_parameter(tpl: OpaqueType, index: usize) -> *const c_char {
    unsafe { template_parameter(tpl, index) }
}

/// # Safety
/// `tpl` must be a valid `reflect_template` that is no longer in use.
pub unsafe fn destroy_template(tpl: OpaqueType) {
    unsafe {
        template_destroy(tpl);
    }
}

/// # Safety
/// `tpl` must be a valid function template and `args` must point to a valid
/// array of template arguments with `size` elements.
pub unsafe fn instantiate_template_function(
    tpl: OpaqueType,
    args: &mut [TemplateArgument],
) -> OpaqueType {
    unsafe { template_instantiate_function(tpl, args.as_mut_ptr() as OpaqueType, args.len()) }
}

/// # Safety
/// `loader_impl` must be a valid loader implementation and `name` must
/// identify a valid MetaCall type.
pub unsafe fn get_loader_type(loader_impl: OpaqueType, name: &str) -> OpaqueType {
    let name = CString::new(name).expect("type name contains NUL");

    unsafe { loader_impl_type(loader_impl, name.as_ptr()) }
}

/// Creates a MetaCall integer value.
///
/// # Safety
/// The returned opaque pointer is owned by the caller and must be used
/// according to the MetaCall value API.
pub unsafe fn create_int_value(value: i32) -> OpaqueType {
    unsafe { value_create_int(value as c_int) }
}

/// Calls a MetaCall function with the provided arguments.
///
/// # Safety
/// `function` must be a valid MetaCall function pointer, and every element
/// of `args` must be a valid MetaCall value pointer compatible with the
/// function signature.
pub unsafe fn call_function(function: OpaqueType, args: &mut [OpaqueType]) -> OpaqueType {
    unsafe { function_call(function, args.as_mut_ptr(), args.len()) }
}

/// Converts a MetaCall value containing an Int into an `i32`.
///
/// # Safety
/// `v` must be a valid MetaCall value containing an integer.
pub unsafe fn int_from_value(v: OpaqueType) -> i32 {
    unsafe { value_to_int(v) }
}

/// # Safety
/// `loader_impl` must be a valid initialized loader implementation pointer.
pub unsafe fn get_loader_lifecycle_state(loader_impl: OpaqueType) -> *mut LoaderLifecycleState {
    (unsafe { loader_impl_get(loader_impl) }) as *mut LoaderLifecycleState
}

static mut RS_LOADER_PTR: *mut c_void = std::ptr::null_mut();

/// # Safety
/// `loader_impl` must be valid and remain alive during registration.
pub unsafe fn loader_lifecycle_register(loader_impl: OpaqueType) {
    let tag = c"rs";
    unsafe {
        loader_initialization_register(loader_impl);

        // Get rust loader pointer
        if RS_LOADER_PTR.is_null() {
            RS_LOADER_PTR = metacall_loader(tag.as_ptr());
        }
    }
}

/// # Safety
/// `loader_impl` must be valid and own the children being unloaded.
pub unsafe fn loader_lifecycle_unload_children(loader_impl: OpaqueType) {
    unsafe {
        loader_unload_children(loader_impl);
    }
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

/// # Safety
/// All pointer arguments must be valid for the duration of type registration.
pub unsafe fn define_type(
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

pub fn rs_loader_destroyed() -> bool {
    unsafe { loader_is_destroyed(RS_LOADER_PTR) == 0 }
}
