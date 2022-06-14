use crate::api::{
    class_singleton, function_singleton, register_class, register_function, ClassCreate,
    ClassRegistration, FunctionCreate, FunctionInputSignature, FunctionRegistration, OpaqueType,
};
use crate::wrapper::class;
use crate::{Class, CompilerState, DlopenLibrary, Function};

fn function_create(func: &Function, dlopen_library: &DlopenLibrary) -> FunctionCreate {
    let name = func.name.clone();
    let args_count = func.args.len();

    let register_func_name = format!("metacall_register_fn_{}", name);
    let register_func: unsafe fn() -> *mut class::NormalFunction =
        unsafe { dlopen_library.instance.symbol(&register_func_name[..]) }.unwrap();
    let function_impl = unsafe { register_func() } as OpaqueType;
    FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton: function_singleton as OpaqueType,
    }
}

fn class_create(class: &Class, dlopen_library: &DlopenLibrary) -> ClassCreate {
    let name = class.name.clone();
    let register_func_name = format!("metacall_register_class_{}", name);
    let register_func: unsafe fn() -> *mut class::Class =
        unsafe { dlopen_library.instance.symbol(&register_func_name[..]) }.unwrap();
    let class_impl = unsafe { register_func() } as OpaqueType;
    ClassCreate {
        name,
        class_impl,
        singleton: class_singleton as OpaqueType,
        class_info: class.clone(),
    }
}

pub fn register(
    state: &CompilerState,
    dlopen_library: &DlopenLibrary,
    loader_impl: OpaqueType,
    ctx: OpaqueType,
) {
    // register functions
    for func in state.functions.iter() {
        let function_registration = FunctionRegistration {
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

    // register classes
    for class in state.classes.iter() {
        let class_registration = ClassRegistration {
            ctx,
            loader_impl,
            class_create: class_create(class, &dlopen_library),
        };
        register_class(class_registration);
    }
}
