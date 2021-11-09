#![feature(rustc_private)]
#![feature(once_cell)]

extern crate rustc_ast;
extern crate rustc_driver;
extern crate rustc_error_codes;
extern crate rustc_errors;
extern crate rustc_hash;
extern crate rustc_attr;
extern crate rustc_hir;
extern crate rustc_interface;
extern crate rustc_session;
extern crate rustc_span;
extern crate rustc_feature;

use rustc_session::config;
use rustc_session::config::CrateType;
use rustc_span::source_map;
use rustc_interface::{Config, Queries, interface::Compiler};
use rustc_ast::{ast, visit, FnKind, Param, NodeId};
use rustc_span::Span;

use std::{error::Error, fs::File, io::Read, path::PathBuf};

pub use syn::{self, File as SynFileAst};

pub mod file;
pub mod package;
pub(crate) mod registrator;

pub enum RegistrationError {
    DlopenError(String),
    ValidationError(String),
    SynError(Box<dyn Error>),
}

pub struct Source {
    code: String,
    dir: Option<PathBuf>,
    filename: PathBuf,
}
impl Source {
    pub fn new(code: String, dir: Option<PathBuf>, filename: PathBuf) -> Source {
        Source { code, dir, filename }
    }
}

#[derive(Debug)]
pub struct Parser {
    pub code: String,
    pub ast: SynFileAst,
}
impl Parser {
    pub fn new(path_to_code: &PathBuf) -> Result<Parser, Box<dyn Error>> {
        let mut file = File::open(path_to_code)?;

        let mut content = String::new();
        file.read_to_string(&mut content)?;

        let code = content.clone();
        let ast: SynFileAst = syn::parse_file(&content)?;

        Ok(Parser { ast, code })
    }
}

fn toolchain_path(home: Option<String>, toolchain: Option<String>) -> Option<PathBuf> {
    home.and_then(|home| {
        toolchain.map(|toolchain| {
            let mut path = PathBuf::from(home);
            path.push("toolchains");
            path.push(toolchain);
            path
        })
    })
}

fn compiler_sys_root() -> Option<PathBuf> {
    // Get the sysroot, looking from most specific to this invocation to the least:
    // - runtime environment
    //    - SYSROOT
    //    - RUSTUP_HOME, MULTIRUST_HOME, RUSTUP_TOOLCHAIN, MULTIRUST_TOOLCHAIN
    // - sysroot from rustc in the path
    // - compile-time environment
    //    - SYSROOT
    //    - RUSTUP_HOME, MULTIRUST_HOME, RUSTUP_TOOLCHAIN, MULTIRUST_TOOLCHAIN
    std::env::var("SYSROOT").ok().map(PathBuf::from)
        .or_else(|| {
            let home = std::env::var("RUSTUP_HOME")
                .or_else(|_| std::env::var("MULTIRUST_HOME"))
                .ok();
            let toolchain = std::env::var("RUSTUP_TOOLCHAIN")
                .or_else(|_| std::env::var("MULTIRUST_TOOLCHAIN"))
                .ok();
            toolchain_path(home, toolchain)
        })
        .or_else(|| option_env!("SYSROOT").map(PathBuf::from))
        .or_else(|| {
            let home = option_env!("RUSTUP_HOME")
                .or(option_env!("MULTIRUST_HOME"))
                .map(ToString::to_string);
            let toolchain = option_env!("RUSTUP_TOOLCHAIN")
                .or(option_env!("MULTIRUST_TOOLCHAIN"))
                .map(ToString::to_string);
            toolchain_path(home, toolchain)
        })
}

fn compiler_source() -> Option<PathBuf> {
    match compiler_sys_root() {
        Some(sys_root) => {
            let mut path = sys_root.clone();
            path.push("lib");
            path.push("rustlib");
            path.push("src");
            path.push("rust");

            if path.exists() {
                return Some(path);
            } else {
                return None;
            }
        },
        _ => None,
    }
}

struct FunctionVisitor {
    functions: Vec<String>,
}

impl FunctionVisitor {
    fn new() -> FunctionVisitor {
        FunctionVisitor {
            functions: vec![],
        }
    }

    fn add_function(&mut self, name: String) {
        self.functions.push(name)
    }
}

// visit::Visitor is the generic trait for walking an AST
impl<'a> visit::Visitor<'a> for FunctionVisitor {
    fn visit_fn(&mut self, fk: visit::FnKind, s: Span, _: NodeId) {
        match fk {
            visit::FnKind::Fn(_, indent, ..) => self.add_function(indent.name.to_string()),
            _ => ()
        }

        visit::walk_fn(self, fk, s)
    }
}

struct CompilerCallbacks {
    source: Source,
    output_path: Option<PathBuf>,
}

impl rustc_driver::Callbacks for CompilerCallbacks {
    fn config(&mut self, config: &mut Config) {
        // Set up inputs
        let name_str = String::from(self.source.filename.clone().to_str().unwrap());

        config.input = config::Input::Str {
            name: source_map::FileName::Custom(name_str.clone()),
            input: self.source.code.clone(),
        };

        config.input_path = Some(match self.source.dir.clone() {
            Some(p) => {
                let mut result = p.clone();
                result.push(name_str.clone());
                result
            },
            None => PathBuf::from(name_str.clone()),
        });

        // Set up outputs
        #[cfg(unix)]
        let lib_extension = "so";
        #[cfg(windows)]
        let lib_extension = "dll";
        #[cfg(macos)]
        let lib_extension = "dylib";

        let mut lib_name = self.source.filename.clone();
        lib_name.set_extension(lib_extension);

        self.output_path = Some(match self.source.dir.clone() {
            Some(p) => {
                let mut result = p.clone();
                result.push(lib_name.clone());
                result
            },
            None => lib_name.clone(),
        });

        config.output_file = self.output_path.clone();

        // Setting up default compiler flags
        config.opts.output_types = rustc_session::config::OutputTypes::new(&[(rustc_session::config::OutputType::Exe, None)]);
        config.opts.optimize = rustc_session::config::OptLevel::Default;
        config.opts.unstable_features = rustc_feature::UnstableFeatures::Allow;
        config.opts.real_rust_source_base_dir = compiler_source();
        config.opts.edition = rustc_span::edition::Edition::Edition2021;
    }

    fn after_analysis<'tcx>(
        &mut self,
        compiler: &Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> rustc_driver::Compilation {
        let krate = queries.parse().expect("no Result<Query<Crate>> found").take();
        let crate_name = match rustc_attr::find_crate_name(compiler.session(), &krate.attrs) {
            Some(name) => name.to_string(),
            None => String::from("unknown_crate"),
        };
        println!("In crate: {},\n", crate_name);

        let mut fn_visitor = FunctionVisitor::new();
        visit::walk_crate(&mut fn_visitor, &krate);

        println!("Found {} functions", fn_visitor.functions.len());

        let functions = fn_visitor.functions.clone();

        println!("{:?}", functions);

        rustc_driver::Compilation::Continue
    }
}

// Buffer diagnostics in a Vec<u8>
#[derive(Clone)]
struct DiagnosticSink(std::sync::Arc<std::sync::Mutex<Vec<u8>>>);

impl std::io::Write for DiagnosticSink {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.0.lock().unwrap().write(buf)
    }
    fn flush(&mut self) -> std::io::Result<()> {
        self.0.lock().unwrap().flush()
    }
}

const BUG_REPORT_URL: &str = "https://github.com/metacall/core/issues/new";

static ICE_HOOK: std::lazy::SyncLazy<Box<dyn Fn(&std::panic::PanicInfo<'_>) + Sync + Send + 'static>> = std::lazy::SyncLazy::new(|| {
    let hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(|info| report_ice(info, BUG_REPORT_URL)));
    hook
});

fn report_ice(info: &std::panic::PanicInfo<'_>, bug_report_url: &str) {
    // Invoke our ICE handler, which prints the actual panic message and optionally a backtrace
    (*ICE_HOOK)(info);

    // Separate the output with an empty line
    eprintln!();

    let emitter = Box::new(rustc_errors::emitter::EmitterWriter::stderr(
        rustc_errors::ColorConfig::Auto,
        None,
        false,
        false,
        None,
        false,
    ));
    let handler = rustc_errors::Handler::with_emitter(true, None, emitter);

    // a .span_bug or .bug call has already printed what
    // it wants to print.
    if !info.payload().is::<rustc_errors::ExplicitBug>() {
        let d = rustc_errors::Diagnostic::new(rustc_errors::Level::Bug, "unexpected panic");
        handler.emit_diagnostic(&d);
    }

    let xs: Vec<std::borrow::Cow<'static, str>> = vec![
        "the compiler unexpectedly panicked. this is a bug.".into(),
        format!("we would appreciate a bug report: {}", bug_report_url).into(),
    ];

    for note in &xs {
        handler.note_without_error(note);
    }

    // If backtraces are enabled, also print the query stack
    let backtrace = std::env::var_os("RUST_BACKTRACE").map_or(false, |x| &x != "0");

    let num_frames = if backtrace { None } else { Some(2) };

    rustc_interface::interface::try_print_query_stack(&handler, num_frames);
}

fn run_compiler(
    callbacks: &mut (dyn rustc_driver::Callbacks + Send),
) -> Result<(), rustc_errors::ErrorReported> {

    // TODO: This must be run only once
    rustc_driver::init_rustc_env_logger();
    std::lazy::SyncLazy::force(&ICE_HOOK);
    // END-TODO

    let diagnostics_buffer = std::sync::Arc::new(std::sync::Mutex::new(Vec::new()));
    let errors_buffer = std::sync::Arc::new(std::sync::Mutex::new(Vec::new()));

    // TODO: Implement different input depending on file or memory:
    // https://doc.rust-lang.org/beta/nightly-rustc/src/rustc_session/config.rs.html#534

    let mut config = Config {
        // Command line options
        opts: config::Options {
            maybe_sysroot: compiler_sys_root(),
            crate_types: vec![CrateType::Cdylib],
            ..config::Options::default()
        },
        // cfg! configuration in addition to the default ones
        crate_cfg: rustc_hash::FxHashSet::default(), // FxHashSet<(String, Option<String>)>
        input: config::Input::File(PathBuf::new()),
        input_path: None,  // Option<PathBuf>
        output_dir: None,  // Option<PathBuf>
        output_file: None, // Option<PathBuf>
        file_loader: None, // Option<Box<dyn FileLoader + Send + Sync>>
        diagnostic_output: rustc_session::DiagnosticOutput::Raw(Box::from(DiagnosticSink(
            diagnostics_buffer.clone(),
        ))),
        // Set to capture stderr output during compiler execution
        stderr: Some(errors_buffer.clone()),                    // Option<Arc<Mutex<Vec<u8>>>>
        lint_caps: rustc_hash::FxHashMap::default(), // FxHashMap<lint::LintId, lint::Level>
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
        registry: rustc_errors::registry::Registry::new(&rustc_error_codes::DIAGNOSTICS),
        make_codegen_backend: None,
    };

    callbacks.config(&mut config);

    let result = rustc_interface::run_compiler(config, |compiler| {
        let sess = compiler.session();

        let linker = compiler.enter(|queries| {
            let early_exit = || sess.compile_status().map(|_| None);

            queries.parse()?;

            if callbacks.after_parsing(compiler, queries) == rustc_driver::Compilation::Stop {
                return early_exit();
            }

            queries.expansion()?;
            if callbacks.after_expansion(compiler, queries) == rustc_driver::Compilation::Stop {
                return early_exit();
            }

            queries.prepare_outputs()?;

            queries.global_ctxt()?;

            queries.global_ctxt()?.peek_mut().enter(|tcx| {
                tcx.analysis(())
            })?;

            if callbacks.after_analysis(compiler, queries) == rustc_driver::Compilation::Stop {
                return early_exit();
            }

            queries.ongoing_codegen()?;

            let linker = queries.linker()?;
            Ok(Some(linker))
        })?;

        if let Some(linker) = linker {
            linker.link()?
        }

        Ok(())
    });

    // Read buffered diagnostics
    let diagnostics = String::from_utf8(diagnostics_buffer.lock().unwrap().clone()).unwrap();
    println!("{}", diagnostics);

    // Read buffered errors
    let errors = String::from_utf8(errors_buffer.lock().unwrap().clone()).unwrap();
    println!("{}", errors);

    result
}

pub fn compile(source: Source) -> Result<PathBuf, String> {
    let mut callbacks = CompilerCallbacks {
        source,
        output_path: None,
    };

    match rustc_driver::catch_fatal_errors(|| {
        run_compiler(&mut callbacks)
    }).and_then(|result| result) {
        Ok(()) => Ok(callbacks.output_path.unwrap()),
        Err(err) => Err(format!("{:?}", err)),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compile() {
        let dir = PathBuf::from(std::env::var("TEST_DEST_DIR").unwrap());
        match compile(Source {
            code: String::from("#[no_mangle]\npub extern \"C\" fn add(num_1: i32, num_2: i32) -> i32 { num_1 + num_2 }"),
            dir: Some(dir),
            filename: PathBuf::from("test.rs"),
        }) {
            Ok(_) => assert!(false, "compilation failed"),
            Err(_) => assert!(true),
        }
    }
}
