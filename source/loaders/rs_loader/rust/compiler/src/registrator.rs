use std::{ffi::c_void, os::raw::c_uint};

use syn::{Attribute, File as SynFileAst, Item, Signature, Visibility};

use api::{
    register_function, FunctionCreate, FunctionInputSignature, FunctionRegisteration,
    PrimitiveMetacallImplTypes,
};

pub fn register(ast: &SynFileAst, loader_impl: *mut c_void, ctx: *mut c_void) {
    for ast_item in ast.items.iter() {
        match ast_item {
            // If the item was a Function
            Item::Fn(function_item) => {
                // If the Function was Public
                match &function_item.vis {
                    Visibility::Public(_) => {}
                    _ => continue,
                }

                // If the Function had #[no_mangle] attribute
                for function_attribute in &function_item.attrs {
                    let Attribute {
                        path: attribute_path,
                        ..
                    } = &function_attribute;

                    let mut no_mangle_is_associated = false;

                    for attribute_segment in attribute_path.segments.iter() {
                        if attribute_segment.ident.to_string() == "no_mangle" {
                            no_mangle_is_associated = true;
                        }
                    }

                    if !no_mangle_is_associated {
                        continue;
                    }
                }

                // If the Function had extern "C" attribute
                let Signature {
                    abi: fucntion_is_abi,
                    ..
                } = &function_item.sig;
                if fucntion_is_abi.is_some() {
                    if let Some(lit_str) = &fucntion_is_abi.as_ref().unwrap().name {
                        if lit_str.value() != "C" {
                            continue;
                        }
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }

                let Signature {
                    ident: function_name_ident,
                    inputs: function_inputs,
                    ..
                } = &function_item.sig;
                let function_name = function_name_ident.to_string();

                let function_create = FunctionCreate {
                    name: function_name,
                    args_count: function_inputs.len(),
                    singleton: 0 as c_uint as *mut c_void,
                    function_impl: 0 as c_uint as *mut c_void,
                };
                let function_input_signature: Vec<FunctionInputSignature> = Vec::new();

                // PrimitiveMetacallImplTypes::Boolean
                let function_return_signature = match &function_item.sig.output {
                    syn::ReturnType::Default => PrimitiveMetacallImplTypes::Boolean,
                    syn::ReturnType::Type(_type_path, the_type) => match *the_type.clone() {
                        syn::Type::Array(_) => todo!(),
                        syn::Type::BareFn(_) => todo!(),
                        syn::Type::Group(_) => todo!(),
                        syn::Type::ImplTrait(_) => todo!(),
                        syn::Type::Infer(_) => todo!(),
                        syn::Type::Macro(_) => todo!(),
                        syn::Type::Never(_) => todo!(),
                        syn::Type::Paren(_) => todo!(),
                        syn::Type::Path(_path) => todo!(),
                        syn::Type::Ptr(_) => PrimitiveMetacallImplTypes::Ptr,
                        syn::Type::Reference(_) => todo!(),
                        syn::Type::Slice(_) => todo!(),
                        syn::Type::TraitObject(_) => todo!(),
                        syn::Type::Tuple(_) => todo!(),
                        syn::Type::Verbatim(_) => todo!(),
                        syn::Type::__TestExhaustive(_) => todo!(),
                    },
                };

                let function_registration = FunctionRegisteration {
                    ctx,
                    loader_impl,
                    function_create,
                    function_input_signature,
                    function_return_signature,
                };

                register_function(function_registration);
            }
            _ => continue,
        }
    }
}
