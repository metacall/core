use crate::{
    bindings::{metacall_load_from_file, metacall_load_from_memory},
    cstring_enum,
    types::MetaCallLoaderError,
};
use std::{
    ffi::CString,
    fmt,
    path::{Path, PathBuf},
    ptr,
};

pub enum Tag {
    C,
    Cobol,
    Crystal,
    CSharp,
    Dart,
    Deno,
    Extension,
    File,
    Java,
    Julia,
    JavaScript,
    JSM,
    Kind,
    LLVM,
    Lua,
    Mock,
    NodeJS,
    Python,
    Ruby,
    RPC,
    Rust,
    TypeScript,
    Wasm,
}

impl fmt::Display for Tag {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Tag::C => write!(f, "c"),
            Tag::Cobol => write!(f, "cob"),
            Tag::Crystal => write!(f, "cr"),
            Tag::CSharp => write!(f, "cs"),
            Tag::Dart => write!(f, "dart"),
            Tag::Deno => write!(f, "deno"),
            Tag::Extension => write!(f, "ext"),
            Tag::File => write!(f, "file"),
            Tag::Java => write!(f, "java"),
            Tag::Julia => write!(f, "jl"),
            Tag::JavaScript => write!(f, "js"),
            Tag::JSM => write!(f, "jsm"),
            Tag::Kind => write!(f, "kind"),
            Tag::LLVM => write!(f, "llvm"),
            Tag::Lua => write!(f, "lua"),
            Tag::Mock => write!(f, "mock"),
            Tag::NodeJS => write!(f, "node"),
            Tag::Python => write!(f, "py"),
            Tag::Ruby => write!(f, "rb"),
            Tag::RPC => write!(f, "rpc"),
            Tag::Rust => write!(f, "rs"),
            Tag::TypeScript => write!(f, "ts"),
            Tag::Wasm => write!(f, "wasm"),
        }
    }
}

/// Loads a script from a single file. Usage example: ...
/// ```
/// // A Nodejs script
/// metacall::load::from_single_file(Tag::NodeJS, "index.js").unwrap();
/// ```
pub fn from_single_file(tag: Tag, script: impl AsRef<Path>) -> Result<(), MetaCallLoaderError> {
    from_file(tag, [script])
}
/// Loads a script from file. Usage example: ...
/// ```
/// // A Nodejs script
/// metacall::load::from_file(Tag::NodeJS, ["index.js", "main.js"]).unwrap();
/// ```
pub fn from_file(
    tag: Tag,
    scripts: impl IntoIterator<Item = impl AsRef<Path>>,
) -> Result<(), MetaCallLoaderError> {
    let c_tag = cstring_enum!(tag, MetaCallLoaderError)?;
    let mut c_script: CString;

    let mut new_scripts: Vec<*const i8> = Vec::new();
    for script in scripts.into_iter() {
        let script_as_pathbuf = PathBuf::from(script.as_ref());
        let script_as_str = script_as_pathbuf.to_str().unwrap();

        if !script_as_pathbuf.exists() {
            return Err(MetaCallLoaderError::FileNotFound(script_as_pathbuf));
        }
        if !script_as_pathbuf.is_file() {
            return Err(MetaCallLoaderError::NotAFileOrPermissionDenied(
                script_as_pathbuf,
            ));
        }

        c_script = cstring_enum!(script_as_str, MetaCallLoaderError)?;

        new_scripts.push(c_script.as_ptr());
    }

    if unsafe {
        metacall_load_from_file(
            c_tag.as_ptr(),
            new_scripts.as_mut_ptr(),
            new_scripts.len(),
            ptr::null_mut(),
        )
    } != 0
    {
        return Err(MetaCallLoaderError::FromFileFailure);
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
pub fn from_memory(tag: Tag, script: impl ToString) -> Result<(), MetaCallLoaderError> {
    let script = script.to_string();
    let c_tag = cstring_enum!(tag, MetaCallLoaderError)?;
    let c_script = cstring_enum!(script, MetaCallLoaderError)?;

    if unsafe {
        metacall_load_from_memory(
            c_tag.as_ptr(),
            c_script.as_ptr(),
            script.len() + 1,
            ptr::null_mut(),
        )
    } != 0
    {
        return Err(MetaCallLoaderError::FromMemoryFailure);
    }

    Ok(())
}
