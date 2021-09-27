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

use rustc_errors::registry;
use rustc_hash::{FxHashMap, FxHashSet};
use rustc_session::config;
use rustc_session::config::CrateType;
use rustc_span::source_map;
use std::rc::Rc;
use std::sync::Mutex;

mod funtions_extractor;

pub struct Source {
    code: String,
    filename: String,
}
impl Source {
    pub fn new(code: String, filename: String) -> Source {
        Source { code, filename }
    }
}

pub fn extract_functions(source: Source) {
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

    let rc = Rc::new(Mutex::new<Option<Vec<funtions_extractor::ParsedRustFunction>>>(None));

    let rc_compiler = Rc::clone(&rc);

    rustc_interface::run_compiler(config, move |compiler| {
        compiler.enter(move |queries| {
            // Parse the program and print the syntax tree.
            let parsed_source: rustc_ast::Crate = queries.parse().unwrap().take();

            let extracted_functions = rc_compiler.lock().unwrap();

            extracted_functions = funtions_extractor::functionss_extractor(parsed_source)

            // tx.send(extracted_functions).unwrap();
        })
    });
}
