use crate::{
    bindings::{metacall_clear, metacall_load_from_file, metacall_load_from_memory},
    cstring_enum,
    types::{self, MetaCallLoaderError},
};
use std::{
    ffi::CString,
    fmt,
    os::raw::c_void,
    path::{Path, PathBuf},
    ptr::null_mut,
};
pub use types::LoaderTag;

impl fmt::Display for LoaderTag {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            LoaderTag::C => write!(f, "c"),
            LoaderTag::Cobol => write!(f, "cob"),
            LoaderTag::Crystal => write!(f, "cr"),
            LoaderTag::CSharp => write!(f, "cs"),
            LoaderTag::Dart => write!(f, "dart"),
            LoaderTag::Deno => write!(f, "deno"),
            LoaderTag::Extension => write!(f, "ext"),
            LoaderTag::File => write!(f, "file"),
            LoaderTag::Java => write!(f, "java"),
            LoaderTag::Julia => write!(f, "jl"),
            LoaderTag::JavaScript => write!(f, "js"),
            LoaderTag::JSM => write!(f, "jsm"),
            LoaderTag::Kind => write!(f, "kind"),
            LoaderTag::LLVM => write!(f, "llvm"),
            LoaderTag::Lua => write!(f, "lua"),
            LoaderTag::Mock => write!(f, "mock"),
            LoaderTag::NodeJS => write!(f, "node"),
            LoaderTag::Python => write!(f, "py"),
            LoaderTag::Ruby => write!(f, "rb"),
            LoaderTag::RPC => write!(f, "rpc"),
            LoaderTag::Rust => write!(f, "rs"),
            LoaderTag::TypeScript => write!(f, "ts"),
            LoaderTag::Wasm => write!(f, "wasm"),
        }
    }
}

pub struct Handle(*mut c_void);

impl Handle {
    pub fn new() -> Self {
        Self(null_mut())
    }

    pub fn from_mut_raw_ptr(&mut self, ptr: *mut c_void) {
        self.0 = ptr
    }

    pub fn as_mut_raw_ptr(&mut self) -> *mut c_void {
        self.0
    }
}

impl Default for Handle {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for Handle {
    fn drop(&mut self) {
        let result = unsafe { metacall_clear(self.0) };

        if result != 0 {
            // Log or handle the error as needed
            eprintln!("Error during cleanup, metacall_clear returned: {}", result);
        }
    }
}

/// Loads a file from a single file. Usage example: ...
/// ```
/// // A Nodejs path
/// metacall::load::from_single_file("node", "index.js").unwrap();
/// ```
pub fn from_single_file(
    tag: LoaderTag,
    path: impl AsRef<Path>,
    handle: Option<&mut Handle>,
) -> Result<(), MetaCallLoaderError> {
    from_file(tag, [path], handle)
}

/// Loads a path from file. Usage example: ...
/// ```
/// // A Nodejs script
/// metacall::load::from_file(Tag::NodeJS, ["index.js", "main.js"]).unwrap();
/// ```
pub fn from_file(
    tag: LoaderTag,
    paths: impl IntoIterator<Item = impl AsRef<Path>>,
    handle: Option<&mut Handle>,
) -> Result<(), MetaCallLoaderError> {
    let c_tag = cstring_enum!(tag, MetaCallLoaderError)?;
    let mut c_path: CString;

    let mut new_paths: Vec<*const i8> = Vec::new();
    for path in paths.into_iter() {
        let path_as_pathbuf = PathBuf::from(path.as_ref());
        let path_as_str = path_as_pathbuf.to_str().unwrap();

        if !path_as_pathbuf.exists() {
            return Err(MetaCallLoaderError::FileNotFound(path_as_pathbuf));
        }
        if !path_as_pathbuf.is_file() {
            return Err(MetaCallLoaderError::NotAFileOrPermissionDenied(
                path_as_pathbuf,
            ));
        }

        c_path = cstring_enum!(path_as_str, MetaCallLoaderError)?;

        new_paths.push(c_path.as_ptr());
    }

    let mut handle_ptr: *mut c_void = null_mut();

    let handle_ref = match handle {
        Some(_) => &mut handle_ptr,
        None => null_mut(),
    };

    if unsafe {
        metacall_load_from_file(
            c_tag.as_ptr(),
            new_paths.as_mut_ptr(),
            new_paths.len(),
            handle_ref,
        )
    } != 0
    {
        return Err(MetaCallLoaderError::FromFileFailure);
    }

    if let Some(handle_ref) = handle {
        handle_ref.from_mut_raw_ptr(handle_ptr);
    }

    Ok(())
}

/// Loads a script from memory. Usage example: ...
/// ```
/// let script = "function greet() { return 'hi there!' }; module.exports = { greet };";
///
/// // A Nodejs script
/// metacall::load::from_memory(Tag::NodeJS, script).unwrap();
/// ```
pub fn from_memory(
    tag: LoaderTag,
    script: impl ToString,
    handle: Option<&mut Handle>,
) -> Result<(), MetaCallLoaderError> {
    let script = script.to_string();
    let c_tag = cstring_enum!(tag, MetaCallLoaderError)?;
    let c_script = cstring_enum!(script, MetaCallLoaderError)?;

    let mut handle_ptr: *mut c_void = null_mut();

    let handle_ref = match handle {
        Some(_) => &mut handle_ptr,
        None => null_mut(),
    };

    if unsafe {
        metacall_load_from_memory(
            c_tag.as_ptr(),
            c_script.as_ptr(),
            script.len() + 1,
            handle_ref,
        )
    } != 0
    {
        return Err(MetaCallLoaderError::FromMemoryFailure);
    }

    if let Some(handle_ref) = handle {
        handle_ref.from_mut_raw_ptr(handle_ptr);
    }

    Ok(())
}
