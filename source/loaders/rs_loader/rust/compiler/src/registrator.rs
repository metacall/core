use crate::api::ClassRegistration;
use crate::api::{
    add_template_parameter, class_singleton, create_template, function_singleton, get_loader_type,
    instantiate_template_function, register_class, register_existing_function, register_function,
    ClassCreate, FunctionCreate, FunctionInputSignature, FunctionRegistration, OpaqueType,
    TemplateArgument, TEMPLATE_ARGUMENT_TYPE, TEMPLATE_TYPE_FUNCTION,
};
use crate::wrapper::class;
use crate::{Class, CompilerState, DynlinkLibrary, Function};
use std::ffi::CString;
//use crate::template::*;

fn function_create(func: &Function, dynlink: &DynlinkLibrary) -> FunctionCreate {
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

pub fn instantiate_template_by_types(
    template: &Function,
    types: &[String],
    tpl: OpaqueType,
    dynlink: &DynlinkLibrary,
    loader_impl: OpaqueType,
) -> OpaqueType {
    instantiate_template(template, types, tpl, dynlink, loader_impl)
}

fn instantiate_template(
    template: &Function,
    types: &[String],
    tpl: OpaqueType,
    dynlink: &DynlinkLibrary,
    loader_impl: OpaqueType,
) -> OpaqueType {
    assert_eq!(
        template.generics.len(),
        types.len(),
        "Template {} expects {} types, got {}",
        template.name,
        template.generics.len(),
        types.len()
    );

    let wrapper_name = template.instantiate_name(types.to_vec());

    let register_func_name = format!("rs_loader_impl_register_fn_{}", wrapper_name);

    let register_func: unsafe extern "C" fn() -> *mut class::Function = unsafe {
        std::mem::transmute(
            dynlink
                .symbol(&register_func_name)
                .unwrap_or_else(|_| panic!("unable to find register function {}", wrapper_name)),
        )
    };

    let function_impl = unsafe { register_func() } as OpaqueType;

    assert!(
        !function_impl.is_null(),
        "failed to create {} implementation",
        wrapper_name
    );

    let names: Vec<CString> = template
        .generics
        .iter()
        .map(|generic| CString::new(generic.as_str()).unwrap())
        .collect();

    let mut args = template
        .generics
        .iter()
        .enumerate()
        .map(|(index, _)| {
            let ty = unsafe { get_loader_type(loader_impl, &types[index]) };

            assert!(
                !ty.is_null(),
                "failed to resolve metacall type {}",
                types[index]
            );

            TemplateArgument {
                name: names[index].as_ptr(),
                kind: TEMPLATE_ARGUMENT_TYPE,
                value: ty,
            }
        })
        .collect::<Vec<_>>();

    unsafe {
        instantiate_template_function(
            tpl,
            &mut args,
            function_impl,
            function_singleton as OpaqueType,
        )
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

        for instantiation in state
            .instantiations
            .iter()
            .filter(|instantiation| instantiation.template == template.name)
        {
            let function = instantiate_template_by_types(
                template,
                &instantiation.types,
                tpl,
                dynlink,
                loader_impl,
            );

            unsafe {
                register_existing_function(ctx, function);
            }
        }
    }

    for instantiation in &state.instantiations {
        let template = state
            .templates
            .iter()
            .find(|template| template.name == instantiation.template)
            .unwrap_or_else(|| panic!("Template {} not found", instantiation.template));

        let tpl = unsafe { create_template(&template.name, TEMPLATE_TYPE_FUNCTION) };

        assert!(
            !tpl.is_null(),
            "failed to create template {}",
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

        instantiate_template_by_types(template, &instantiation.types, tpl, dynlink, loader_impl);
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
