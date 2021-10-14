use rustc_ast::{ast::ItemKind::Fn, Crate, FnKind, FnRetTy, PatKind::Ident, TyKind};
use std::borrow::Borrow;

#[derive(Debug)]
pub struct ParsedRustFunctionArguments {
    name: String,
    order_number: usize,
    value_type: TyKind,
}
#[derive(Debug)]
pub struct ParsedRustFunction {
    name: String,
    asyncness: rustc_ast::Async,
    return_type: FnRetTy,
    arguments_length: usize,
    arguments: Vec<ParsedRustFunctionArguments>,
}

unsafe impl Send for ParsedRustFunction {}
unsafe impl Sync for ParsedRustFunction {}

pub fn parse_functions(parse_source: Crate) -> Vec<ParsedRustFunction> {
    let mut parsed_rust_functions: Vec<ParsedRustFunction> = Vec::new();

    for parsed_item in parse_source.items.iter() {
        if let Fn(function) = parsed_item.kind.borrow() {
            match function.borrow() {
                FnKind(_the_final, function_signature, _generic, _some) => {
                    let mut arguments: Vec<ParsedRustFunctionArguments> = Vec::new();

                    let mut i: usize = 0;
                    for function_input in function_signature.decl.inputs.iter() {
                        arguments.push(ParsedRustFunctionArguments {
                            order_number: i,
                            name: match function_input.pat.kind {
                                Ident(_by_value, name, _) => name.to_string(),
                                _ => String::new(),
                            },
                            value_type: function_input.ty.kind.clone(),
                        });

                        i += 1;
                    }

                    let return_type = function_signature.decl.output.clone();

                    parsed_rust_functions.push(ParsedRustFunction {
                        arguments,
                        return_type,
                        name: parsed_item.ident.to_string(),
                        asyncness: function_signature.header.asyncness,
                        arguments_length: function_signature.decl.inputs.len(),
                    });
                }
            }
        }
    }

    parsed_rust_functions
}
