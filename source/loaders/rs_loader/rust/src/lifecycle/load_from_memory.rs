use super::loader::{self, LoadingMethod};
use compiler::TemplateInstantiation;
use compiler::{memory::MemoryRegistration, RegistrationError};
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};

#[repr(C)]
struct RsLoaderTemplateInstantiation {
    template_name: *const c_char,
    types: *const *const c_char,
    type_count: usize,
}

#[repr(C)]
struct RsLoaderLoadData {
    instantiations: *const RsLoaderTemplateInstantiation,
    instantiation_count: usize,
}

unsafe fn template_instantiations_from_data(
    data: *mut c_void,
) -> Result<Vec<TemplateInstantiation>, String> {
    if data.is_null() {
        return Ok(Vec::new());
    }

    let data = &*(data as *const RsLoaderLoadData);

    if data.instantiation_count == 0 {
        return Ok(Vec::new());
    }

    if data.instantiations.is_null() {
        return Err("Template instantiation count is nonzero, but the array is null.".to_owned());
    }

    let instantiations = std::slice::from_raw_parts(data.instantiations, data.instantiation_count);

    instantiations
        .iter()
        .enumerate()
        .map(|(index, instantiation)| {
            if instantiation.template_name.is_null() {
                return Err(format!(
                    "Template instantiation at index {} has a null template name.",
                    index
                ));
            }

            let template = CStr::from_ptr(instantiation.template_name)
                .to_str()
                .map_err(|_| {
                    format!(
                        "Template instantiation at index {} has an invalid UTF-8 template name.",
                        index
                    )
                })?
                .to_owned();

            if instantiation.type_count > 0 && instantiation.types.is_null() {
                return Err(format!(
                    "Template '{}' has a nonzero type count, but its type array is null.",
                    template
                ));
            }

            let types = if instantiation.type_count == 0 {
                Vec::new()
            } else {
                std::slice::from_raw_parts(instantiation.types, instantiation.type_count)
                    .iter()
                    .enumerate()
                    .map(|(type_index, ty)| {
                        if ty.is_null() {
                            return Err(format!(
                                "Template '{}' has a null type at index {}.",
                                template, type_index
                            ));
                        }

                        CStr::from_ptr(*ty)
                            .to_str()
                            .map(str::to_owned)
                            .map_err(|_| {
                                format!(
                                    "Template '{}' has an invalid UTF-8 type at index {}.",
                                    template, type_index
                                )
                            })
                    })
                    .collect::<Result<Vec<_>, String>>()?
            };

            Ok(TemplateInstantiation { template, types })
        })
        .collect()
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    name: *const c_char,
    buffer: *const c_char,
    _size: usize,
    data: *mut c_void,
) -> *mut c_void {
    if name.is_null() {
        return loader::load_on_error("Rust loader received a null name pointer.");
    }

    if buffer.is_null() {
        return loader::load_on_error("Rust loader received a null buffer pointer.");
    }

    let name = match CStr::from_ptr(name).to_str() {
        Ok(name) => name.to_owned(),
        Err(_) => return loader::load_on_error("Rust loader received an invalid UTF-8 name."),
    };

    let code = match CStr::from_ptr(buffer).to_str() {
        Ok(code) => code.to_owned(),
        Err(_) => return loader::load_on_error("Rust loader received invalid UTF-8 source code."),
    };
    let instantiations = match template_instantiations_from_data(data) {
        Ok(instantiations) => instantiations,
        Err(error) => return loader::load_on_error(error),
    };
    let instance =
        LoadingMethod::Memory(match MemoryRegistration::new(name, code, &instantiations) {
            Ok(instance) => instance,
            Err(error) => match error {
                RegistrationError::CompilationError(analysis_error) => {
                    return loader::load_on_error(analysis_error);
                }
                RegistrationError::DynlinkError(dynlink_error) => {
                    return loader::load_on_error(dynlink_error);
                }
            },
        });
    Box::into_raw(Box::new(vec![instance])) as *mut c_void
}
