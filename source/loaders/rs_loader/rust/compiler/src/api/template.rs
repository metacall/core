use std::{
    ffi::{CStr, CString},
    os::raw::c_int,
};

use super::*;
use crate::wrapper::{build_generic_mapping, class, substitute_type};
use crate::{Dynlink, DynlinkLibrary, DynlinkSymbolAddr, Function};

#[repr(C)]
pub struct TemplateInterface {
    create: extern "C" fn(OpaqueType, OpaqueType) -> c_int,
    instantiate_function: extern "C" fn(OpaqueType, OpaqueType, OpaqueType, usize) -> OpaqueType,
    instantiate_class: extern "C" fn(OpaqueType, OpaqueType, OpaqueType, usize) -> OpaqueType,
    destroy: extern "C" fn(OpaqueType, OpaqueType),
}

/// Implementation of a template (template_impl), it contains the parsed generic
/// function and the library where its instantiations have been compiled
pub struct TemplateImpl {
    template: Function,
    loader_impl: OpaqueType,
    dynlink: Dynlink,
}

impl TemplateImpl {
    pub fn new(template: Function, loader_impl: OpaqueType, dynlink: &DynlinkLibrary) -> Self {
        TemplateImpl {
            template,
            loader_impl,
            dynlink: dynlink.instance,
        }
    }

    fn symbol(&self, symbol_name: &str) -> Option<DynlinkSymbolAddr> {
        let c_symbol_name = CString::new(symbol_name).ok()?;
        let mut symbol_address = std::mem::MaybeUninit::<DynlinkSymbolAddr>::uninit();

        let result = unsafe {
            crate::dynlink_symbol(
                self.dynlink,
                c_symbol_name.as_ptr(),
                symbol_address.as_mut_ptr(),
            )
        };

        if result == 0 {
            Some(unsafe { symbol_address.assume_init() })
        } else {
            None
        }
    }

    fn instantiate_function(&self, types: Vec<String>) -> OpaqueType {
        // Name of the instantiation, it is the same as the Rust syntax (i.e: identity::<f64>)
        let name = self.template.instantiate(types.clone()).name;

        // Name of the generated wrapper, it must be a valid Rust identifier (i.e: identity_f64)
        let wrapper_name = self.template.instantiate_name(types.clone());

        let register_func_name = format!("rs_loader_impl_register_fn_{}", wrapper_name);

        // The instantiation has not been generated when compiling the source
        let register_func: unsafe extern "C" fn() -> *mut class::Function =
            match self.symbol(&register_func_name) {
                Some(symbol) => unsafe {
                    std::mem::transmute::<
                        DynlinkSymbolAddr,
                        unsafe extern "C" fn() -> *mut class::Function,
                    >(symbol)
                },
                None => return std::ptr::null_mut(),
            };

        let function_impl = unsafe { register_func() } as OpaqueType;

        if function_impl.is_null() {
            return std::ptr::null_mut();
        }

        // The signature is obtained from the template arguments and return, substituting the generics
        let mapping = build_generic_mapping(&self.template, &types);

        let input = self
            .template
            .args
            .iter()
            .map(|arg| FunctionInputSignature {
                name: arg.name.clone(),
                t: substitute_type(arg, &mapping),
            })
            .collect::<Vec<_>>();

        let ret = self
            .template
            .ret
            .as_ref()
            .map(|ret| substitute_type(ret, &mapping));

        let function_create = FunctionCreate {
            name,
            args_count: input.len(),
            function_impl,
            singleton: function_singleton as OpaqueType,
        };

        unsafe { create_function_with_signature(self.loader_impl, function_create, ret, &input) }
    }
}

#[no_mangle]
extern "C" fn template_singleton_create(_tpl: OpaqueType, _tpl_impl: OpaqueType) -> c_int {
    0
}

#[no_mangle]
extern "C" fn template_singleton_instantiate_function(
    _tpl: OpaqueType,
    tpl_impl: OpaqueType,
    args_p: OpaqueType,
    size: usize,
) -> OpaqueType {
    if tpl_impl.is_null() || (args_p.is_null() && size > 0) {
        return std::ptr::null_mut();
    }

    let template_impl = unsafe { &*(tpl_impl as *const TemplateImpl) };
    let args = unsafe { std::slice::from_raw_parts(args_p as *const TemplateArgument, size) };
    let mut types = Vec::with_capacity(size);

    for arg in args {
        // TODO: Template arguments (nested templates) are not supported yet
        if arg.kind != TEMPLATE_ARGUMENT_TYPE {
            return std::ptr::null_mut();
        }

        let name = unsafe { type_name(arg.value) };

        if name.is_null() {
            return std::ptr::null_mut();
        }

        match unsafe { CStr::from_ptr(name) }.to_str() {
            Ok(name) => types.push(name.to_owned()),
            Err(_) => return std::ptr::null_mut(),
        }
    }

    template_impl.instantiate_function(types)
}

#[no_mangle]
extern "C" fn template_singleton_instantiate_class(
    _tpl: OpaqueType,
    _tpl_impl: OpaqueType,
    _args_p: OpaqueType,
    _size: usize,
) -> OpaqueType {
    // TODO: Generic structs are not supported yet
    std::ptr::null_mut()
}

#[no_mangle]
extern "C" fn template_singleton_destroy(_tpl: OpaqueType, tpl_impl: OpaqueType) {
    if !rs_loader_destroyed() && !tpl_impl.is_null() {
        unsafe {
            let template_impl = Box::from_raw(tpl_impl as *mut TemplateImpl);
            drop(template_impl);
        }
    }
}

#[no_mangle]
pub extern "C" fn template_singleton() -> *const TemplateInterface {
    static SINGLETON: TemplateInterface = TemplateInterface {
        create: template_singleton_create,
        instantiate_function: template_singleton_instantiate_function,
        instantiate_class: template_singleton_instantiate_class,
        destroy: template_singleton_destroy,
    };

    &SINGLETON
}

/// Registers the template in the context, the value holds the reference
/// to the template, so it is destroyed along with the context
///
/// # Safety
/// `ctx` must be a valid MetaCall context and `tpl` must be a valid template.
pub unsafe fn register_template(ctx: OpaqueType, tpl: OpaqueType) -> c_int {
    let sp = context_scope(ctx);
    let v = value_create_template(tpl);

    if v.is_null() {
        return 1;
    }

    let result = scope_define(sp, get_template_name(tpl) as *mut std::os::raw::c_char, v);

    if result != 0 {
        value_type_destroy(v);
    }

    result
}
