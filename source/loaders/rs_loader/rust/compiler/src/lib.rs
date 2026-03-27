#![feature(rustc_private)]
// Allow us to match on Box<T>s:
#![feature(box_patterns)]
// Allow us to get file prefix
#![feature(path_file_prefix)]
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

use rustc_ast::{visit, Impl, Item, ItemKind, VariantData};
use rustc_hir::def::{DefKind, Res};
use rustc_hir::def_id::DefId;
use rustc_interface::{interface::Compiler, Config, Queries};
use rustc_middle::ty::{TyCtxt, Visibility};
use rustc_session::config::{
    self, CrateType, ErrorOutputType, ExternEntry, ExternLocation, Externs, Input,
};
use rustc_session::search_paths::SearchPath;
use rustc_span::FileName;
use std::io::Write;
use std::iter::{self, FromIterator};
use std::{
    collections::{BTreeMap, BTreeSet, HashMap},
    fmt,
    path::{Path, PathBuf},
    sync,
};

use std::ffi::CString;
use std::os::raw::c_char;

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
    DynlinkError(String),
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

#[allow(clippy::new_ret_no_self)]
impl Source {
    pub fn new(source: Source) -> SourceImpl {
        let library_name = |file_name: &PathBuf| {
            #[cfg(unix)]
            let lib_extension = "so";
            #[cfg(windows)]
            let lib_extension = "dll";
            #[cfg(target_os = "macos")]
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
                        .unwrap_or_else(|| panic!("Unable to get the parent of {:?}", path)),
                );
                let name = PathBuf::from(
                    path.file_name()
                        .unwrap_or_else(|| panic!("Unable to get the parent of {:?}", path)),
                );
                let temp_dir = std::env::temp_dir();
                SourceImpl {
                    input: SourceInput(config::Input::File(path.clone())),
                    input_path: input_path(&dir, &name),
                    output: output_path(&temp_dir, &name),
                    source,
                }
            }
            Source::Memory { ref name, ref code } => {
                let dir = std::env::temp_dir();
                let name_path = PathBuf::from(name.clone());

                SourceImpl {
                    input: SourceInput(config::Input::Str {
                        name: FileName::Custom(name.clone()),
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
                        .unwrap_or_else(|| panic!("Unable to get the parent of {:?}", path)),
                );
                let name = PathBuf::from(
                    path.file_name()
                        .unwrap_or_else(|| panic!("Unable to get the filename of {:?}", path)),
                );
                let temp_dir = std::env::temp_dir();
                SourceImpl {
                    input: SourceInput(config::Input::File(path.clone())),
                    input_path: input_path(&dir, &name),
                    output: output_path(&temp_dir, &name),
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
            let mut path = sys_root;
            path.push("lib");
            path.push("rustlib");
            path.push("src");
            path.push("rust");

            if path.exists() {
                Some(path)
            } else {
                None
            }
        }
        _ => None,
    }
}

// Define the opaque pointer (C type: dynlink)
type Dynlink = *mut std::ffi::c_void;
type DynlinkSymbolAddr = unsafe extern "C" fn();

// Define flags as constants (mimicking the C #define values)
// const DYNLINK_FLAGS_BIND_NOW: u32 = 0x01 << 0;       // Inmediate loading bind flag
const DYNLINK_FLAGS_BIND_LAZY: u32 = 0x01 << 1; // Lazy loading bind flag
const DYNLINK_FLAGS_BIND_LOCAL: u32 = 0x01 << 2; // Private visibility bind flag
                                                 // const DYNLINK_FLAGS_BIND_GLOBAL: u32 = 0x01 << 3;    // Public visibility bind flag
                                                 // const DYNLINK_FLAGS_BIND_SELF: u32 = 0x01 << 16;     // Private flag for when loading the current process

// FFI declaration for the `dynlink_load_absolute` function (C function)
extern "C" {
    pub fn dynlink_load_absolute(path: *const c_char, flags: u32) -> Dynlink;
    pub fn dynlink_symbol(
        dynlink: Dynlink,
        symbol_name: *const c_char,
        symbol_address: *mut DynlinkSymbolAddr,
    ) -> i32;
    pub fn dynlink_unload(dynlink: Dynlink);
}

#[derive(Debug)]
pub struct DynlinkLibrary {
    pub instance: Dynlink,
}

impl DynlinkLibrary {
    pub fn new(path: &Path) -> Result<DynlinkLibrary, String> {
        let c_path = CString::new(path.to_str().unwrap()).expect("CString::new failed");

        unsafe {
            let instance = dynlink_load_absolute(
                c_path.as_ptr(),
                DYNLINK_FLAGS_BIND_LOCAL | DYNLINK_FLAGS_BIND_LAZY,
            );

            if instance.is_null() {
                Err("Failed to load library: ".to_owned() + path.to_str().unwrap())
            } else {
                Ok(DynlinkLibrary { instance })
            }
        }
    }

    pub fn symbol(&self, symbol_name: &str) -> Result<DynlinkSymbolAddr, String> {
        let c_symbol_name = CString::new(symbol_name).expect("CString::new failed");

        unsafe {
            let mut symbol_address = std::mem::MaybeUninit::<DynlinkSymbolAddr>::uninit();

            let result = dynlink_symbol(
                self.instance,
                c_symbol_name.as_ptr(),
                symbol_address.as_mut_ptr(),
            );
            if result == 0 {
                Ok(symbol_address.assume_init())
            } else {
                Err(format!("Failed to find symbol: {}", symbol_name))
            }
        }
    }
}

impl Drop for DynlinkLibrary {
    fn drop(&mut self) {
        unsafe {
            if !self.instance.is_null() {
                // Unload the dynamic library
                dynlink_unload(self.instance);

                // Set the instance to null for safety
                self.instance = std::ptr::null_mut();
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
    This, // Self in struct method
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
        if self.args.is_empty() {
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
    destructor: Option<Function>, // TODO: Maybe we don't need destructor, just drop the variable (review)
    methods: Vec<Function>,
    static_methods: Vec<Function>,
    attributes: Vec<Attribute>,
    // static_attributes: Vec<Attribute>, // We don't handle static attrs in rust
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
    destination: PathBuf,
    functions: Vec<Function>,
    classes: Vec<Class>,
}

impl CompilerCallbacks {
    // Updated: analyze_source takes &Crate directly
    fn analyze_source(&mut self, krate: &rustc_ast::Crate) {
        let mut item_visitor = ItemVisitor::new();
        visit::walk_crate(&mut item_visitor, krate);
        self.classes = item_visitor.classes.into_values().collect();
        self.functions = item_visitor.functions;
    }

    // Updated: analyze_metadata takes TyCtxt directly
    fn analyze_metadata<'tcx>(&mut self, tcx: TyCtxt<'tcx>) {
        let mut class_map: HashMap<DefId, Class> = HashMap::new();
        let krates = tcx.crates(());
        // Since we are loading a package, input_path should be lib<crate_name>.rlib
        let crate_name = &self
            .source
            .input_path
            .file_prefix()
            .expect("Unable to get file prefix.")
            .to_str()
            .expect("Unable to cast OsStr to str")[3..];
        // Find our crate
        let crate_num = krates
            .iter()
            .find(|&&x| tcx.crate_name(x) == rustc_span::Symbol::intern(crate_name))
            .or_else(|| krates.get(0))
            .expect("unable to find crate");
        // Parse public functions and structs
        // Updated: use tcx.module_children() and ModChild instead of Export
        for child in tcx.module_children(crate_num.as_def_id()) {
            let ident = child.ident;
            let res = child.res;
            let vis = child.vis;
            // Skip non-public items
            if !matches!(vis, Visibility::Public) {
                continue;
            }
            match res {
                Res::Def(DefKind::Struct, def_id) => {
                    let class = class_map.entry(def_id).or_default();
                    class.name = ident.to_string();

                    for field in tcx.module_children(def_id) {
                        if let Some(field) =
                            middle::extract_attribute_from_mod_child(&tcx, field)
                        {
                            class.attributes.push(field);
                        }
                    }

                    // inherent_impls returns &[DefId] in nightly-2024-11-01
                    let inherent_impls = tcx.inherent_impls(def_id);
                    for inherent_impl in inherent_impls {
                        for method in tcx.module_children(*inherent_impl) {
                            if let Some(function) =
                                middle::extract_fn_from_mod_child(&tcx, method)
                            {
                                if function.name == "new" {
                                    class.constructor = Some(function);
                                } else if function.has_self() {
                                    class.methods.push(function);
                                } else {
                                    class.static_methods.push(function);
                                }
                            }
                        }
                    }
                }
                Res::Def(DefKind::Fn, def_id) => {
                    // Updated: fn_sig now returns EarlyBinder, skip_binder() to unwrap
                    let fn_sig = tcx.fn_sig(def_id).skip_binder();
                    let names = tcx.fn_arg_names(def_id);
                    self.functions.push(middle::handle_fn(
                        ident.to_string(),
                        &fn_sig,
                        names,
                    ));
                }
                _ => {}
            }
        }
        // After parsing all structs, parse trait implementations
        for &trait_impl in tcx.trait_impls_in_crate(*crate_num) {
            use rustc_middle::ty::fast_reject::SimplifiedType;
            if tcx.def_kind(trait_impl) != (DefKind::Impl { of_trait: true }) {
                continue;
            }
            let self_ty_opt = tcx.impl_trait_ref(trait_impl)
                .and_then(|tr| {
                    let self_ty = tr.skip_binder().self_ty();
                    rustc_middle::ty::fast_reject::simplify_type(tcx, self_ty, rustc_middle::ty::fast_reject::TreatParams::InstantiateWithInfer)
                });
            if let Some(SimplifiedType::Adt(def_id)) = self_ty_opt {
                if let Some(class) = class_map.get_mut(&def_id) {
                    for func in tcx.module_children(trait_impl) {
                        if let Some(function) = middle::extract_fn_from_mod_child(&tcx, func)
                        {
                            if function.name == "drop" {
                                class.destructor = Some(function);
                            } else if function.has_self() {
                                class.methods.push(function);
                            } else {
                                class.static_methods.push(function);
                            }
                        }
                    }
                }
            }
        }
        self.classes = class_map.into_values().collect();
    }
}

static CHARSET_STR: &str = "abcdefghijklmnopqrstuvwxyz";
static METACALL_STR: &str = "metacall-";

fn generate_random_string(length: usize) -> String {
    let chars: Vec<char> = CHARSET_STR.chars().collect();
    let mut result = String::with_capacity(length + METACALL_STR.len());

    result.push_str(METACALL_STR);

    for _ in 0..length {
        let random_index = fastrand::usize(..chars.len());
        result.push(chars[random_index]);
    }

    result
}

impl rustc_driver::Callbacks for CompilerCallbacks {
    fn config(&mut self, config: &mut Config) {
        if matches!(self.source.source, Source::Package { .. }) {
            let mut externs: BTreeMap<String, ExternEntry> = BTreeMap::new();
            let name = "metacall_package";
            let path = self.source.input_path.clone();
            let canon_path = rustc_session::utils::CanonicalizedPath::new(&path);

            let entry = externs.entry(name.to_owned());

            use std::collections::btree_map::Entry;

            match entry {
                Entry::Vacant(vacant) => {
                    let files = BTreeSet::from_iter(iter::once(canon_path));
                    vacant.insert(ExternEntry {
                        location: ExternLocation::ExactPaths(files),
                        is_private_dep: false,
                        add_prelude: true,
                        nounused_dep: false,
                        force: false,
                    });
                }
                _ => {
                    unimplemented!();
                }
            }

            config.opts.externs = Externs::new(externs);

            // We hardcode the dependency path for now
            let dep_path = self
                .source
                .input_path
                .clone()
                .parent()
                .expect("Unable to get parent dir")
                .join("deps");
            let early_dcx = rustc_session::EarlyDiagCtxt::new(ErrorOutputType::default());
            config.opts.search_paths.push(SearchPath::from_cli_opt(
                &std::env::current_dir().unwrap_or_default(),
                &config.opts.target_triple,
                &early_dcx,
                format!("dependency={}", dep_path.display()).as_str(),
                false,
            ));
            // Set up inputs
            let wrapped_script_path = self.destination.join("metacall_wrapped_package.rs");
            if self.is_parsing {
                let mut wrapped_script = std::fs::File::create(&wrapped_script_path)
                    .expect("unable to create wrapped script");
                wrapped_script
                    .write_all("extern crate metacall_package;\n".as_bytes())
                    .expect("Unablt to write wrapped script");
            }

            config.input = Input::File(wrapped_script_path.clone());
        } else {
            // Set up inputs
            config.input = self.source.input.clone().0;
        }
        // Set up output
        if self.is_parsing {
            let output_path = self.source.output.clone();
            let file_name = output_path.file_name().expect("Unable to get the filename");
            config.output_file = Some(config::OutFileName::Real(self.destination.join(file_name)));
            self.source.output = self.destination.join(file_name);
        } else {
            config.output_file = Some(config::OutFileName::Real(self.source.output.clone()));
        }
        // Setting up default compiler flags
        config.opts.output_types = config::OutputTypes::new(&[(config::OutputType::Exe, None)]);
        config.opts.optimize = config::OptLevel::Default;
        config.opts.unstable_features = rustc_feature::UnstableFeatures::Allow;
        config.opts.real_rust_source_base_dir = compiler_source();
        config.opts.edition = rustc_span::edition::Edition::Edition2021;
    }

    fn after_crate_root_parsing<'tcx>(
        &mut self,
        _compiler: &Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> rustc_driver::Compilation {
        if self.is_parsing {
            match self.source.source {
                Source::File { .. } | Source::Memory { .. } => {
                    let krate = queries.parse().expect("Failed to parse");
                    let krate_ref = krate.borrow();
                    self.analyze_source(&krate_ref);
                    return rustc_driver::Compilation::Stop;
                }
                _ => {}
            }
        }
        rustc_driver::Compilation::Continue
    }

    fn after_expansion<'tcx>(
        &mut self,
        _compiler: &Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> rustc_driver::Compilation {
        if self.is_parsing {
            match self.source.source {
                Source::Package { .. } => {
                    let gcx = queries.global_ctxt().expect("Failed to get global context");
                    gcx.borrow().enter(|tcx| {
                        self.analyze_metadata(tcx);
                    });
                    return rustc_driver::Compilation::Stop;
                }
                _ => {}
            }
        }
        rustc_driver::Compilation::Continue
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
            ItemKind::Struct(VariantData::Struct { fields, .. }, _) => {
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
                    if let rustc_ast::AssocItemKind::Fn(box rustc_ast::Fn { sig, .. }) = &item.kind
                    {
                        // Function has self in parameters
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
                            // Static method
                            match &sig.decl.output {
                                rustc_ast::FnRetTy::Ty(p) => {
                                    let rustc_ast::Ty { kind, .. } = &**p;

                                    match kind {
                                        rustc_ast::TyKind::Path(_, p) => {
                                            let ret_name = p.segments[0].ident.to_string();
                                            if ret_name == "Self" || ret_name == class_name_str {
                                                class.constructor = Some(ast::handle_fn(name, sig));
                                            } else {
                                                class
                                                    .static_methods
                                                    .push(ast::handle_fn(name, sig));
                                            }
                                        }
                                        _ => {
                                            class.static_methods.push(ast::handle_fn(name, sig));
                                        }
                                    }
                                }
                                rustc_ast::FnRetTy::Default(_) => {
                                    class.static_methods.push(ast::handle_fn(name, sig));
                                }
                            }
                        }
                    }
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

// Updated: use std::sync::LazyLock instead of std::lazy::SyncLazy
static ICE_HOOK: std::sync::LazyLock<
    Box<dyn Fn(&std::panic::PanicHookInfo<'_>) + Sync + Send + 'static>,
> = std::sync::LazyLock::new(|| {
    let hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(|info| report_ice(info, BUG_REPORT_URL)));
    hook
});

// Updated: PanicInfo renamed to PanicHookInfo in newer Rust
fn report_ice(info: &std::panic::PanicHookInfo<'_>, bug_report_url: &str) {
    // Invoke our ICE handler, which prints the actual panic message and optionally a backtrace
    (*ICE_HOOK)(info);

    // Separate the output with an empty line
    eprintln!();

    // Updated: simplified ICE reporting — just print to stderr
    eprintln!("the compiler unexpectedly panicked. this is a bug.");
    eprintln!("we would appreciate a bug report: {}", bug_report_url);

    // If backtraces are enabled, also print the query stack
    let _backtrace = std::env::var_os("RUST_BACKTRACE").map_or(false, |x| &x != "0");
}

// Updated: using_internal_features now requires Arc<AtomicBool>
static USING_INTERNAL_FEATURES: std::sync::LazyLock<std::sync::Arc<std::sync::atomic::AtomicBool>> =
    std::sync::LazyLock::new(|| std::sync::Arc::new(std::sync::atomic::AtomicBool::new(true)));

pub fn initialize() {
    std::sync::LazyLock::force(&ICE_HOOK);
}

fn run_compiler(
    callbacks: &mut (dyn rustc_driver::Callbacks + Send),
    diagnostics_buffer: &sync::Arc<sync::Mutex<Vec<u8>>>,
    _errors_buffer: &sync::Arc<sync::Mutex<Vec<u8>>>,
) -> Result<(), rustc_errors::ErrorGuaranteed> {
    let mut config = Config {
        opts: config::Options {
            maybe_sysroot: compiler_sys_root(),
            crate_types: vec![CrateType::Cdylib],
            ..Default::default()
        },
        crate_cfg: Vec::new(),
        crate_check_cfg: Vec::new(),
        input: config::Input::File(PathBuf::new()),
        output_dir: None,
        output_file: None,
        ice_file: None,
        file_loader: None,
        locale_resources: Vec::new(),
        lint_caps: rustc_hash::FxHashMap::default(),
        psess_created: None,
        hash_untracked_state: None,
        register_lints: None,
        override_queries: None,
        make_codegen_backend: None,
        registry: rustc_errors::registry::Registry::new(rustc_errors::DIAGNOSTICS),
        using_internal_features: USING_INTERNAL_FEATURES.clone(),
        expanded_args: Vec::new(),
    };

    callbacks.config(&mut config);

    rustc_interface::run_compiler(config, |compiler| {
        compiler.enter(|queries| {
            queries.global_ctxt()?.borrow().enter(|tcx| {
                tcx.analysis(())
            })
        })
    })
}

pub fn compile(source: SourceImpl) -> Result<CompilerState, CompilerError> {
    let destination = std::env::temp_dir().join(generate_random_string(5));
    let result = std::fs::create_dir(&destination);

    // Handle the case that tempdir doesn't exist
    if result.is_err() {
        let destination = source.input_path.join(generate_random_string(10));
        std::fs::create_dir(&destination).expect("Unable to create temp folder");
    }

    let mut callbacks = CompilerCallbacks {
        source,
        is_parsing: true,
        destination,
        functions: Default::default(),
        classes: Default::default(),
    };

    let diagnostics_buffer = sync::Arc::new(sync::Mutex::new(Vec::new()));
    let errors_buffer = sync::Arc::new(sync::Mutex::new(Vec::new()));

    // Parse and generate wrapper
    let parsing_result: Result<(), CompilerError> = match run_compiler(&mut callbacks, &diagnostics_buffer, &errors_buffer)
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

    // Parse fails, stop
    if let Err(e) = parsing_result {
        return Err(e);
    }

    let mut patched_callback = generate_wrapper(callbacks).expect("Unable to generate wrapper");

    // Generate binary
    match run_compiler(&mut patched_callback, &diagnostics_buffer, &errors_buffer)
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
                code: String::from("pub fn add(a: i32, b: i32) -> i32 { a + b }"),
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
