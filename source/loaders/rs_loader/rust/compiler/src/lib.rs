#![feature(rustc_private)]
#![feature(once_cell)]
// allow us to match on Box<T>s:
#![feature(box_patterns)]
#![feature(let_else)]
#![feature(iter_zip)]
extern crate rustc_ast;
extern crate rustc_ast_pretty;
extern crate rustc_attr;
extern crate rustc_driver;
extern crate rustc_error_codes;
extern crate rustc_errors;
extern crate rustc_feature;
extern crate rustc_hash;
extern crate rustc_hir;
extern crate rustc_interface;
extern crate rustc_middle;
extern crate rustc_session;
extern crate rustc_span;

use dlopen;
use rustc_ast::{visit, Impl, Item, ItemKind, VariantData};
use rustc_hir::def::{DefKind, Res};
use rustc_hir::def_id::DefId;
use rustc_interface::{interface::Compiler, Config, Queries};
use rustc_middle::hir::exports::Export;
use rustc_middle::ty::Visibility;
use rustc_session::config::{self, CrateType, ExternEntry, ExternLocation, Externs, Input};
use rustc_session::utils::CanonicalizedPath;
use rustc_span::source_map;
use std::io::Write;
use std::iter::{self, FromIterator};
use std::{
    collections::{BTreeMap, BTreeSet, HashMap},
    fmt,
    path::PathBuf,
    sync,
};
mod ast;
pub mod file;
pub mod memory;
mod middle;
pub mod package;
pub(crate) mod registrator;
pub mod wrapper;
use wrapper::generate_wrapper;
pub mod api;
pub enum RegistrationError {
    CompilationError(String),
    DlopenError(String),
}

struct SourceInput(config::Input);

impl Clone for SourceInput {
    fn clone(&self) -> Self {
        match &self.0 {
            config::Input::File(path) => SourceInput(config::Input::File(path.clone())),
            config::Input::Str { name, input } => SourceInput(config::Input::Str {
                name: name.clone(),
                input: input.clone(),
            }),
        }
    }
}

pub struct SourceImpl {
    input: SourceInput,
    input_path: PathBuf,
    output: PathBuf,
    source: Source,
}

#[derive(Clone)]
pub enum Source {
    File { path: PathBuf },
    Memory { name: String, code: String },
    Package { path: PathBuf },
}

impl Source {
    pub fn new(source: Source) -> SourceImpl {
        let library_name = |file_name: &PathBuf| {
            #[cfg(unix)]
            let lib_extension = "so";
            #[cfg(windows)]
            let lib_extension = "dll";
            #[cfg(macos)]
            let lib_extension = "dylib";

            let mut lib_name = file_name.clone();
            lib_name.set_extension(lib_extension);

            lib_name
        };

        let input_path = |dir: &PathBuf, name: &PathBuf| {
            let mut path = PathBuf::from(dir);
            path.push(name);
            path.clone()
        };

        let output_path = |dir: &PathBuf, name: &PathBuf| input_path(dir, &library_name(name));

        match source {
            Source::File { ref path } => {
                let dir = PathBuf::from(
                    path.clone()
                        .parent()
                        .expect(format!("Unable to get the parent of {:?}", path).as_str()),
                );
                let name = PathBuf::from(
                    path.file_name()
                        .expect(format!("Unable to get the filename of {:?}", path).as_str()),
                );

                SourceImpl {
                    input: SourceInput(config::Input::File(path.clone())),
                    input_path: input_path(&dir, &name),
                    output: output_path(&dir, &name),
                    source,
                }
            }
            Source::Memory { ref name, ref code } => {
                let dir = PathBuf::from(std::env::temp_dir());
                let name_path = PathBuf::from(name.clone());

                SourceImpl {
                    input: SourceInput(config::Input::Str {
                        name: source_map::FileName::Custom(name.clone()),
                        input: code.clone(),
                    }),
                    input_path: input_path(&dir, &name_path),
                    output: output_path(&dir, &name_path),
                    source,
                }
            }
            Source::Package { ref path } => {
                let dir = PathBuf::from(
                    path.clone()
                        .parent()
                        .expect(format!("Unable to get the parent of {:?}", path).as_str()),
                );
                let name = PathBuf::from(
                    path.file_name()
                        .expect(format!("Unable to get the filename of {:?}", path).as_str()),
                );

                SourceImpl {
                    input: SourceInput(config::Input::File(path.clone())),
                    input_path: input_path(&dir, &name),
                    output: output_path(&dir, &name),
                    source,
                }
            }
        }
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
    std::env::var("SYSROOT")
        .ok()
        .map(PathBuf::from)
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
        }
        _ => None,
    }
}

#[derive(Debug)]
pub struct DlopenLibrary {
    pub instance: dlopen::raw::Library,
}
impl DlopenLibrary {
    pub fn new(path_to_dll: &PathBuf) -> Result<DlopenLibrary, String> {
        match match dlopen::raw::Library::open(path_to_dll.clone()) {
            Ok(instance) => return Ok(DlopenLibrary { instance }),
            Err(error) => match error {
                dlopen::Error::NullCharacter(null_error) => {
                    Err(format!(
                        "Provided string could not be coverted into `{}` because it contained null character. IoError: {}",
                        "std::ffi::CString",
                        null_error
                    ))
                }
                dlopen::Error::OpeningLibraryError(io_error) => {
                    Err(format!(
                        "The dll could not be opened. IoError: {}",
                        io_error
                    ))
                }
                dlopen::Error::SymbolGettingError(io_error) => {
                    Err(format!(
                        "The symbol could not be obtained. IoError: {}",
                        io_error
                    ))
                }
                dlopen::Error::NullSymbol => {
                    Err(format!(
                        "Value of the symbol was null.",
                    ))
                }
                dlopen::Error::AddrNotMatchingDll(io_error) => {
                    Err(format!(
                        "Address could not be matched to a dynamic link library. IoError: {}",
                        io_error
                    ))
                }
            },
        } {
            Ok(dlopen_library_instance) => return Ok(dlopen_library_instance),
            Err(error) => {
                let dll_opening_error = format!(
                    "{}\nrs_loader was unable to open the dll with the following path: `{}`", 
                    error,
                    path_to_dll.to_str().expect("Unable to cast pathbuf to str")
                );

                return Err(dll_opening_error)
            }
        }
    }
}

#[derive(Clone, Debug)]
pub enum Mutability {
    Yes,
    No,
}
#[derive(Clone, Debug)]
pub enum Reference {
    Yes,
    No,
}

#[allow(non_camel_case_types)]
#[derive(Clone, Debug)]
pub enum FunctionType {
    i16,
    i32,
    i64,
    u16,
    u32,
    u64,
    usize,
    f32,
    f64,
    bool,
    char,
    Array,
    Map,
    Slice,
    str,
    String,
    Ptr,
    Null,
    Complex,
    This, // self in struct method
}

impl fmt::Display for FunctionType {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        fmt.write_str(format!("{:?}", self).as_str())?;
        Ok(())
    }
}

#[derive(Clone, Debug)]
pub struct FunctionParameter {
    name: String,
    mutability: Mutability,
    reference: Reference,
    ty: FunctionType,
    generic: Vec<FunctionParameter>,
}

#[derive(Clone, Debug, Default)]
pub struct Function {
    name: String,
    ret: Option<FunctionParameter>,
    args: Vec<FunctionParameter>,
}

impl Function {
    pub fn has_self(&self) -> bool {
        if self.args.len() == 0 {
            return false;
        }
        self.args[0].name == "self"
    }
}

#[derive(Clone, Debug)]
pub struct Attribute {
    name: String,
    ty: FunctionParameter,
}

#[derive(Clone, Debug, Default)]
pub struct Class {
    name: String,
    constructor: Option<Function>,
    destructor: Option<Function>, // maybe we don't need destructor. just drop the variable
    methods: Vec<Function>,
    static_methods: Vec<Function>,
    attributes: Vec<Attribute>,
    // static_attributes: Vec<Attribute>, // we don't handle static attrs in rust
}
#[derive(Clone, Debug)]
pub struct CompilerState {
    output: PathBuf,
    functions: Vec<Function>,
    classes: Vec<Class>,
}

#[derive(Clone, Debug)]
pub struct CompilerError {
    diagnostics: String,
    errors: String,
    err: String,
}

pub struct CompilerCallbacks {
    source: SourceImpl,
    is_parsing: bool,
    functions: Vec<Function>,
    classes: Vec<Class>,
}

impl CompilerCallbacks {
    fn analyze_source<'tcx>(&mut self, queries: &'tcx Queries<'tcx>) {
        let krate = queries
            .parse()
            .expect("no Result<Query<Crate>> found")
            .take();
        let mut item_visitor = ItemVisitor::new();
        visit::walk_crate(&mut item_visitor, &krate);
        self.classes = item_visitor.classes.into_values().collect();
        self.functions = item_visitor.functions;
    }
    fn analyze_metadata<'tcx>(&mut self, queries: &'tcx Queries<'tcx>) {
        let mut class_map: HashMap<DefId, Class> = HashMap::new();
        let crate_num = queries
            .expansion()
            .expect("Unable to get Expansion")
            .peek_mut()
            .1
            .borrow_mut()
            .access(|resolver| {
                let c_store = resolver.cstore().clone();
                c_store
                    .crates_untracked()
                    .last()
                    .cloned()
                    .expect("Unable to get last element of crates.")
            });
        queries
            .global_ctxt()
            .expect("Unable to get global ctxt")
            .peek_mut()
            .enter(|ctxt| {
                // parse public functions and structs
                for child in ctxt.item_children(crate_num.as_def_id()) {
                    let Export {
                        ident, res, vis, ..
                    } = child;
                    // skip non-public items
                    if !matches!(vis, Visibility::Public) {
                        continue;
                    }
                    match res {
                        Res::Def(DefKind::Struct, def_id) => {
                            let class = class_map.entry(*def_id).or_default();
                            class.name = ident.to_string();

                            for field in ctxt.item_children(*def_id) {
                                if let Some(field) =
                                    middle::extract_attribute_from_export(&ctxt, field)
                                {
                                    class.attributes.push(field);
                                }
                            }

                            for inherent_impl in ctxt.inherent_impls(*def_id) {
                                for method in ctxt.item_children(*inherent_impl) {
                                    if let Some(function) =
                                        middle::extract_fn_from_export(&ctxt, method)
                                    {
                                        if function.name == "new" {
                                            class.constructor = Some(function);
                                        } else {
                                            if function.has_self() {
                                                class.methods.push(function);
                                            } else {
                                                class.static_methods.push(function);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Res::Def(DefKind::Fn, def_id) => {
                            // https://doc.rust-lang.org/stable/nightly-rustc/rustc_middle/ty/struct.Binder.html
                            let fn_sig = ctxt.fn_sig(*def_id);
                            let names = ctxt.fn_arg_names(*def_id);
                            self.functions.push(middle::handle_fn(
                                ident.to_string(),
                                &fn_sig,
                                names,
                            ));
                        }
                        _ => {}
                    }
                }
                // after parsing all structs, parse tarit implementations.
                for trait_impl in ctxt.all_trait_implementations(crate_num) {
                    use rustc_middle::ty::fast_reject::SimplifiedTypeGen::AdtSimplifiedType;
                    if let Some(AdtSimplifiedType(def_id)) = trait_impl.1 {
                        if let Some(class) = class_map.get_mut(&def_id) {
                            for func in ctxt.item_children(trait_impl.0) {
                                if let Some(function) = middle::extract_fn_from_export(&ctxt, func)
                                {
                                    if function.name == "drop" {
                                        class.destructor = Some(function);
                                    } else {
                                        if function.has_self() {
                                            class.methods.push(function);
                                        } else {
                                            class.static_methods.push(function);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            });
        self.classes = class_map.into_values().collect();
    }
}

impl rustc_driver::Callbacks for CompilerCallbacks {
    fn config(&mut self, config: &mut Config) {
        if matches!(self.source.source, Source::Package { .. }) {
            let mut externs: BTreeMap<String, ExternEntry> = BTreeMap::new();
            let name = "metacall_package";
            let path = self.source.input_path.clone();
            let path = CanonicalizedPath::new(&path);

            let entry = externs.entry(name.to_owned());

            use std::collections::btree_map::Entry;

            match entry {
                Entry::Vacant(vacant) => {
                    let files = BTreeSet::from_iter(iter::once(path));
                    vacant.insert(ExternEntry {
                        location: ExternLocation::ExactPaths(files),
                        is_private_dep: false,
                        add_prelude: true,
                    });
                }
                _ => {
                    unimplemented!();
                }
            }

            config.opts.externs = Externs::new(externs);
            // Set up inputs
            let wrapped_script_path = self
                .source
                .input_path
                .clone()
                .parent()
                .expect("input path has no parent")
                .join("metacall_wrapped_package.rs");
            if self.is_parsing {
                let mut wrapped_script = std::fs::File::create(&wrapped_script_path)
                    .expect("unable to create wrapped script");
                wrapped_script
                    .write("extern crate metacall_package;\n".as_bytes())
                    .expect("Unablt to write wrapped script");
            }

            config.input = Input::File(wrapped_script_path.clone()); // self.source.input.clone().0;
            config.input_path = Some(wrapped_script_path);
        } else {
            // Set up inputs
            config.input = self.source.input.clone().0;
            config.input_path = Some(self.source.input_path.clone());
        }
        // Set up output
        config.output_file = Some(self.source.output.clone());

        // Setting up default compiler flags
        config.opts.output_types = config::OutputTypes::new(&[(config::OutputType::Exe, None)]);
        config.opts.optimize = config::OptLevel::Default;
        config.opts.unstable_features = rustc_feature::UnstableFeatures::Allow;
        config.opts.real_rust_source_base_dir = compiler_source();
        config.opts.edition = rustc_span::edition::Edition::Edition2021;
    }

    fn after_expansion<'tcx>(
        &mut self,
        _compiler: &Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> rustc_driver::Compilation {
        // analysis
        // is_parsing will be set to false after generating wrappers.
        if self.is_parsing {
            match self.source.source {
                Source::File { .. } | Source::Memory { .. } => {
                    self.analyze_source(queries);
                    rustc_driver::Compilation::Stop
                }
                Source::Package { .. } => {
                    self.analyze_metadata(queries);
                    rustc_driver::Compilation::Stop
                }
            }
        } else {
            // we have finished the parsing process.
            rustc_driver::Compilation::Continue
        }
    }
}

enum ImplKind {
    Drop,
    None,
    Other,
}

struct ItemVisitor {
    functions: Vec<Function>,
    classes: HashMap<String, Class>,
}

impl ItemVisitor {
    fn new() -> Self {
        Self {
            functions: vec![],
            classes: HashMap::new(),
        }
    }
}

// visit::Visitor is the generic trait for walking an AST
impl<'a> visit::Visitor<'a> for ItemVisitor {
    fn visit_item(&mut self, i: &Item) {
        match &i.kind {
            ItemKind::Struct(VariantData::Struct(fields, _), _) => {
                // let def_id = i.
                let class = self.classes.entry(i.ident.to_string()).or_default();
                class.name = i.ident.to_string();
                for field in fields {
                    if let Some(ident) = field.ident {
                        let attr = Attribute {
                            name: ident.to_string(),
                            ty: ast::handle_ty(&field.ty),
                        };
                        class.attributes.push(attr);
                    }
                }
            }
            ItemKind::Impl(box impl_kind) => {
                let Impl {
                    items,
                    self_ty,
                    of_trait,
                    ..
                } = impl_kind;
                let impl_kind = match of_trait {
                    None => ImplKind::None,
                    Some(of_trait) => {
                        let of_trait_name = of_trait.path.segments[0].ident.to_string();
                        if of_trait_name == "Drop" {
                            ImplKind::Drop
                        } else {
                            ImplKind::Other
                        }
                    }
                };
                let class_name = match &self_ty.kind {
                    rustc_ast::TyKind::Path(_, path) => path.segments[0].ident.to_string(),
                    _ => unreachable!(),
                };
                let class = self.classes.entry(class_name.clone()).or_default();
                let class_name_str = class_name.as_str();

                for item in items {
                    let name = item.ident.to_string();
                    match &item.kind {
                        rustc_ast::AssocItemKind::Fn(box rustc_ast::Fn { sig, .. }) => {
                            // function has self in parameters
                            if sig.decl.has_self() {
                                match impl_kind {
                                    ImplKind::Drop => {
                                        class.destructor = Some(ast::handle_fn(name, sig));
                                    }
                                    _ => {
                                        class.methods.push(ast::handle_fn(name, sig));
                                    }
                                }
                            } else {
                                // static method
                                match &sig.decl.output {
                                    rustc_ast::FnRetTy::Ty(p) => match &**p {
                                        rustc_ast::Ty { kind, .. } => match kind {
                                            rustc_ast::TyKind::Path(_, p) => {
                                                let ret_name = p.segments[0].ident.to_string();
                                                if ret_name == "Self" || ret_name == class_name_str
                                                {
                                                    class.constructor =
                                                        Some(ast::handle_fn(name, sig));
                                                } else {
                                                    class
                                                        .static_methods
                                                        .push(ast::handle_fn(name, sig));
                                                }
                                            }
                                            _ => {
                                                class
                                                    .static_methods
                                                    .push(ast::handle_fn(name, sig));
                                            }
                                        },
                                    },
                                    rustc_ast::FnRetTy::Default(_) => {
                                        class.static_methods.push(ast::handle_fn(name, sig));
                                    }
                                }
                            }
                        }
                        _ => {}
                    };
                }
            }
            ItemKind::Fn(box sig) => {
                self.functions
                    .push(ast::handle_fn(i.ident.to_string(), &sig.sig));
            }
            _ => {}
        }
    }
}

// Buffer diagnostics in a Vec<u8>
#[derive(Clone)]
struct DiagnosticSink(sync::Arc<sync::Mutex<Vec<u8>>>);

impl std::io::Write for DiagnosticSink {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.0.lock().expect("Unable to acquire lock").write(buf)
    }
    fn flush(&mut self) -> std::io::Result<()> {
        self.0.lock().expect("Unable to acquire lock").flush()
    }
}

const BUG_REPORT_URL: &str = "https://github.com/metacall/core/issues/new";

static ICE_HOOK: std::lazy::SyncLazy<
    Box<dyn Fn(&std::panic::PanicInfo<'_>) + Sync + Send + 'static>,
> = std::lazy::SyncLazy::new(|| {
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

    // a .span_bug or .bug call has already printed what it wants to print
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

pub fn initialize() {
    rustc_driver::init_rustc_env_logger();
    std::lazy::SyncLazy::force(&ICE_HOOK);
}

fn run_compiler(
    callbacks: &mut (dyn rustc_driver::Callbacks + Send),
    diagnostics_buffer: &sync::Arc<sync::Mutex<Vec<u8>>>,
    errors_buffer: &sync::Arc<sync::Mutex<Vec<u8>>>,
) -> Result<(), rustc_errors::ErrorReported> {
    let mut config = Config {
        // Command line options
        opts: config::Options {
            maybe_sysroot: compiler_sys_root(),
            crate_types: vec![CrateType::Cdylib],
            ..Default::default()
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
        stderr: Some(errors_buffer.clone()), // Option<Arc<Mutex<Vec<u8>>>>
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

    rustc_interface::run_compiler(config, |compiler| {
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

            queries
                .global_ctxt()?
                .peek_mut()
                .enter(|tcx| tcx.analysis(()))?;

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
    })
}

pub fn compile(source: SourceImpl) -> Result<CompilerState, CompilerError> {
    let mut callbacks = CompilerCallbacks {
        source,
        is_parsing: true,
        functions: Default::default(),
        classes: Default::default(),
    };

    let diagnostics_buffer = sync::Arc::new(sync::Mutex::new(Vec::new()));
    let errors_buffer = sync::Arc::new(sync::Mutex::new(Vec::new()));

    // parse and generate wrapper
    let parsing_result: Result<(), CompilerError> = match rustc_driver::catch_fatal_errors(|| {
        run_compiler(&mut callbacks, &diagnostics_buffer, &errors_buffer)
    })
    .and_then(|result| result)
    {
        Ok(()) => Ok(()),
        Err(err) => {
            // Read buffered diagnostics
            let diagnostics = String::from_utf8(
                diagnostics_buffer
                    .lock()
                    .expect("Unable to acquire lock")
                    .clone(),
            )
            .expect("Unable to get string from utf8");
            eprintln!("{}", diagnostics);

            // Read buffered errors
            let errors = String::from_utf8(
                errors_buffer
                    .lock()
                    .expect("Unable to acquire lock")
                    .clone(),
            )
            .expect("Unable to get string from utf8");
            eprintln!("{}", errors);

            return Err(CompilerError {
                diagnostics,
                errors,
                err: format!("{:?}", err),
            });
        }
    };
    // parse fails, stop
    if let Err(e) = parsing_result {
        return Err(e);
    }

    let mut patched_callback = generate_wrapper(callbacks).expect("Unable to generate wrapper");

    // generate binary
    match rustc_driver::catch_fatal_errors(|| {
        run_compiler(&mut patched_callback, &diagnostics_buffer, &errors_buffer)
    })
    .and_then(|result| result)
    {
        Ok(()) => Ok(CompilerState {
            output: patched_callback.source.output.clone(),
            functions: patched_callback.functions,
            classes: patched_callback.classes,
        }),
        Err(err) => {
            // Read buffered diagnostics
            let diagnostics = String::from_utf8(
                diagnostics_buffer
                    .lock()
                    .expect("Unable to acquire lock")
                    .clone(),
            )
            .expect("Unable to get string from utf8");
            eprintln!("{}", diagnostics);

            // Read buffered errors
            let errors = String::from_utf8(
                errors_buffer
                    .lock()
                    .expect("Unable to acquire lock")
                    .clone(),
            )
            .expect("Unable to get string from utf8");
            eprintln!("{}", errors);

            Err(CompilerError {
                diagnostics,
                errors,
                err: format!("{:?}", err),
            })
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Once;

    static INIT: Once = Once::new();

    fn run_test<T>(test: T) -> ()
    where
        T: FnOnce() -> () + std::panic::UnwindSafe,
    {
        INIT.call_once(|| {
            // Initialize the compiler
            initialize();
        });
        let result = std::panic::catch_unwind(|| test());
        assert!(result.is_ok())
    }

    #[test]
    fn test_compile_memory() {
        run_test(|| {
            match compile(Source::new(Source::Memory {
                name: String::from("test.rs"),
                code: String::from(
                    "#[no_mangle]\npub extern \"C\" fn add(a: i32, b: i32) -> i32 { a + b }",
                ),
            })) {
                Err(comp_err) => assert!(false, "compilation failed: {}", comp_err.errors),
                Ok(comp_state) => assert!(comp_state.output.exists()),
            }
        })
    }

    #[test]
    fn test_compile_file() {
        run_test(|| {
            match compile(Source::new(Source::File {
                path: PathBuf::from(std::env::var("TEST_SOURCE_DIR").unwrap()),
            })) {
                Err(comp_err) => assert!(false, "compilation failed: {}", comp_err.errors),
                Ok(comp_state) => assert!(comp_state.output.exists()),
            }
        })
    }
}
