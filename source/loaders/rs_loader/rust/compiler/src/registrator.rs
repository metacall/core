use crate::api::{
    add_template_parameter, call_function, class_singleton, create_function, create_int_value,
    create_template, function_singleton, get_loader_type, instantiate_template_function,
    int_from_value, register_class, register_function, ClassCreate, ClassRegistration,
    FunctionCreate, FunctionInputSignature, FunctionRegistration, OpaqueType, TemplateArgument,
    TEMPLATE_ARGUMENT_TYPE, TEMPLATE_TYPE_FUNCTION,
};
use crate::wrapper::class;
use crate::{Class, CompilerState, DynlinkLibrary, Function};
use std::ffi::CString;
//use crate::template::*;

fn function_create(func: &Function, dynlink: &DynlinkLibrary) -> FunctionCreate {
    println!(
        "Creating MetaCall function: {}, generics: {:?}",
        func.name, func.generics
    );

    let name = func.name.clone();
    let args_count = func.args.len();
    let register_func_name = format!("rs_loader_impl_register_fn_{}", name);
    let register_func: unsafe extern "C" fn() -> *mut class::Function = unsafe {
        std::mem::transmute(
            dynlink
                .symbol(&register_func_name[..])
                .unwrap_or_else(|_| panic!("Unable to find register function {}", name)),
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
    let register_func_name = format!("rs_loader_impl_register_class_{}", name);
    let register_func: unsafe extern "C" fn() -> *mut class::Class = unsafe {
        std::mem::transmute(
            dynlink
                .symbol(&register_func_name[..])
                .unwrap_or_else(|_| panic!("Unable to find register function {}", name)),
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
    for func in state.functions.iter() {
        println!("Function: {}, Generics: {:?}", func.name, func.generics);

        let function_registration = FunctionRegistration {
            ctx,
            loader_impl,
            function_create: function_create(func, dynlink),
            ret: func.ret.as_ref().map(|ret| ret.ty.to_string()),
            input: func
                .args
                .iter()
                .map(|param| FunctionInputSignature {
                    name: param.name.clone(),
                    t: param.ty.to_string(),
                })
                .collect(),
            template: None,
        };

        register_function(function_registration);
    }

    for template in state.templates.iter() {

        let tpl = unsafe { create_template(&template.name, TEMPLATE_TYPE_FUNCTION) };

        assert!(
            !tpl.is_null(),
            "Failed to create template {}",
            template.name
        );

        for generic in &template.generics {
            let result = unsafe { add_template_parameter(tpl, generic) };

            assert_eq!(
                result, 0,
                "Failed to add generic parameter {} to {}",
                generic, template.name
            );
        }

        if template.name == "identity" {
            let int_type = unsafe { get_loader_type(loader_impl, "i32") };

            assert!(!int_type.is_null(), "Failed to resolve MetaCall i32 type");

            let parameter_name = CString::new("T").expect("template parameter contains NUL");

            let mut args = [TemplateArgument {
                name: parameter_name.as_ptr(),
                kind: TEMPLATE_ARGUMENT_TYPE,
                value: int_type,
            }];

            let instantiated = unsafe { instantiate_template_function(tpl, &mut args) };

            assert!(
                !instantiated.is_null(),
                "Failed to instantiate template {}",
                template.name
            );

            let register_func_name = "rs_loader_impl_register_fn_identity_i32";

            let register_func: unsafe extern "C" fn() -> *mut class::Function = unsafe {
                std::mem::transmute(
                    dynlink
                        .symbol(register_func_name)
                        .expect("Unable to find register function identity_i32"),
                )
            };

            let function_impl = unsafe { register_func() } as OpaqueType;

            assert!(
                !function_impl.is_null(),
                "Failed to create identity_i32 implementation"
            );

            let concrete_function = unsafe {
                create_function(
                    "identity_i32",
                    1,
                    function_impl,
                    function_singleton as OpaqueType,
                )
            };

            assert!(
                !concrete_function.is_null(),
                "Failed to create concrete identity_i32 function"
            );

            let input = unsafe { create_int_value(42) };

            assert!(!input.is_null(), "Failed to create i32 value");

            let mut call_args = [input];

            let result = unsafe { call_function(concrete_function, &mut call_args) };

            assert!(
                !result.is_null(),
                "Failed to call instantiated template {}",
                template.name
            );

            let returned = unsafe { int_from_value(result) };

            println!("identity<i32>(42) returned {}", returned);

            assert_eq!(returned, 42);
        }
    }

    // Register classes
    for class in state.classes.iter() {
        let class_registration = ClassRegistration {
            ctx,
            loader_impl,
            class_create: class_create(class, dynlink),
        };

        register_class(class_registration);
    }
}
