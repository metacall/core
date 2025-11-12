use crate::api::{
    class_singleton, function_singleton, register_class, register_function, ClassCreate,
    ClassRegistration, FunctionCreate, FunctionInputSignature, FunctionRegistration, OpaqueType,
};
use crate::wrapper::class;
use crate::{Class, CompilerState, DynlinkLibrary, Function};

fn function_create(func: &Function, dynlink: &DynlinkLibrary) -> FunctionCreate {
    let name = func.name.clone();
    let args_count = func.args.len();

    let register_func_name = format!("metacall_register_fn_{}", name);
    let register_func: unsafe extern "C" fn() -> *mut class::NormalFunction =
        unsafe {
            std::mem::transmute(
                dynlink.symbol(&register_func_name[..])
                    .expect(format!("Unable to find register function {}", name).as_str())
            )
        };
    let function_impl = unsafe { register_func() } as OpaqueType;

    FunctionCreate {
        name,
        args_count,
        function_impl,
        singleton: function_singleton as OpaqueType,
    }
}

fn class_create(class: &Class, dynlink: &DynlinkLibrary) -> ClassCreate {
    let name = class.name.clone();
    let register_func_name = format!("metacall_register_class_{}", name);
    let register_func: unsafe extern "C" fn() -> *mut class::Class =
        unsafe {
            std::mem::transmute(
                dynlink.symbol(&register_func_name[..])
                    .expect(format!("Unable to find register function {}", name).as_str())
            )
        };
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
    dynlink: &DynlinkLibrary,
    loader_impl: OpaqueType,
    ctx: OpaqueType,
) {
    // register functions
    for func in state.functions.iter() {
        let function_registration = FunctionRegistration {
            ctx,
            loader_impl,
            function_create: function_create(func, &dynlink),
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
            class_create: class_create(class, &dynlink),
        };
        register_class(class_registration);
    }
}
