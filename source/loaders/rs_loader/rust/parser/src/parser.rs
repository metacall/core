use rustc_ast::ast::ItemKind::Fn;
use rustc_ast::Crate;
use rustc_ast::FnKind;
use rustc_ast::FnRetTy::Ty;
use rustc_ast::PatKind::Ident;
use rustc_ast::TyKind::Path;
use rustc_span::symbol::Ident as TypeIdent;
use std::borrow::Borrow;

#[derive(Debug)]
pub enum ParsedRustFunctionReturnType {
	TypeIdent(TypeIdent),
	Void,
}
#[derive(Debug)]
pub enum ParsedRustFunctionArgumentType {
	TypeIdent(TypeIdent),
	Void,
}
#[derive(Debug)]
pub struct ParsedRustFunctionArguments {
	name: String,
	order_number: usize,
	value_type: ParsedRustFunctionArgumentType,
}
#[derive(Debug)]
pub struct ParsedRustFunction {
	name: String,
	asyncness: rustc_ast::Async,
	return_type: ParsedRustFunctionReturnType,
	arguments_length: usize,
	arguments: Vec<ParsedRustFunctionArguments>,
}

pub fn parse(parse_source: Crate) -> Vec<ParsedRustFunction> {
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
							value_type: match &function_input.borrow().ty.kind {
								Path(_, path) => ParsedRustFunctionArgumentType::TypeIdent(
									path.segments[0].ident,
								),
								_ => ParsedRustFunctionArgumentType::Void,
							},
						});

						i += 1;
					}

					let return_type = match function_signature.decl.output.borrow() {
						Ty(ty) => {
							if let Path(_, path) = &ty.borrow().kind {
								ParsedRustFunctionReturnType::TypeIdent(path.segments[0].ident)
							} else {
								ParsedRustFunctionReturnType::Void
							}
						}
						_ => ParsedRustFunctionReturnType::Void,
					};

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
