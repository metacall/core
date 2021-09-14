#![feature(rustc_private)]
extern crate rustc_ast;
extern crate rustc_driver;
extern crate rustc_error_codes;
extern crate rustc_errors;
extern crate rustc_hash;
extern crate rustc_hir;
extern crate rustc_interface;
extern crate rustc_session;
extern crate rustc_span;

use rustc_ast::ast::ItemKind::Fn;
use rustc_ast::FnKind;
use rustc_ast::FnRetTy::Ty;
use rustc_ast::PatKind::Ident;
use rustc_ast::TyKind::Path;
use rustc_driver::{Callbacks, RunCompiler};
use rustc_errors::registry;
use rustc_hash::{FxHashMap, FxHashSet};
use rustc_session::config;
use rustc_session::config::CrateType;
use rustc_span::source_map;
use rustc_span::symbol::Ident as TypeIdent;
use std::borrow::Borrow;

pub struct Source {
    code: String,
    filename: String,
}
impl Source {
    pub fn new(code: String, filename: String) -> Source {
        Source { code, filename }
    }
}

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
struct Calls;
impl Callbacks for Calls {
    fn config(&mut self, _config: &mut rustc_interface::interface::Config) {}
}

pub fn compile(source: Source) {
    let Source { code, filename } = source;

    let config = rustc_interface::Config {
        // Command line options
        opts: config::Options {
            crate_types: vec![CrateType::Cdylib],
            ..config::Options::default()
        },
        // cfg! configuration in addition to the default ones
        crate_cfg: FxHashSet::default(), // FxHashSet<(String, Option<String>)>
        input: config::Input::Str {
            name: source_map::FileName::Custom(filename.clone()),
            input: code.clone(),
        },
        input_path: None,  // Option<PathBuf>
        output_dir: None,  // Option<PathBuf>
        output_file: None, // Option<PathBuf>
        file_loader: None, // Option<Box<dyn FileLoader + Send + Sync>>
        diagnostic_output: rustc_session::DiagnosticOutput::Default,
        // Set to capture stderr output during compiler execution
        stderr: None,                    // Option<Arc<Mutex<Vec<u8>>>>
        lint_caps: FxHashMap::default(), // FxHashMap<lint::LintId, lint::Level>
        // This is a callback from the driver that is called when [`ParseSess`] is created.
        parse_sess_created: None, //Option<Box<dyn FnOnce(&mut ParseSess) + Send>>
        // This is a callback from the driver that is called when we're registering lints;
        // it is called during plugin registration when we have the LintStore in a non-shared state.
        //
        // Note that if you find a Some here you probably want to call that function in the new
        // function being registered.
        register_lints: None, // Option<Box<dyn Fn(&Session, &mut LintStore) + Send + Sync>>
        // This is a callback from the driver that is called just after we have populated
        // the list of queries.
        //
        // The second parameter is local providers and the third parameter is external providers.
        override_queries: None, // Option<fn(&Session, &mut ty::query::Providers<'_>, &mut ty::query::Providers<'_>)>
        // Registry of diagnostics codes.
        registry: registry::Registry::new(&rustc_error_codes::DIAGNOSTICS),
        make_codegen_backend: None,
    };

    rustc_interface::run_compiler(config, |compiler| {
        compiler.enter(|queries| {
            // Parse the program and print the syntax tree.
            let parse = queries.parse().unwrap().take();
            let mut parsed_rust_functions: Vec<ParsedRustFunction> = Vec::new();

            {
                for parsed_item in parse.items.iter() {
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
                                            Path(_, path) => {
                                                ParsedRustFunctionArgumentType::TypeIdent(
                                                    path.segments[0].ident,
                                                )
                                            }
                                            _ => ParsedRustFunctionArgumentType::Void,
                                        },
                                    });

                                    i += 1;
                                }

                                let return_type = match function_signature.decl.output.borrow() {
                                    Ty(ty) => {
                                        if let Path(_, path) = &ty.borrow().kind {
                                            ParsedRustFunctionReturnType::TypeIdent(
                                                path.segments[0].ident,
                                            )
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

                println!("Results: {:#?}", parsed_rust_functions);
            }
        });
    });

    let args: Vec<_> = std::env::args().collect();
    let mut calls = Calls {};

    RunCompiler::new(&args, &mut calls).run().unwrap();
}
