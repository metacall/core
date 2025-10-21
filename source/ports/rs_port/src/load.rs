use crate::{
    bindings::{metacall_load_from_file, metacall_load_from_memory},
    cstring_enum,
    types::MetaCallLoaderError,
};
use std::{
    ffi::CString,
    path::{Path, PathBuf},
    ptr,
};

/// Loads a script from a single file. Usage example: ...
/// ```
/// // A Nodejs script
/// metacall::load::from_single_file("node", "index.js").unwrap();
/// ```
pub fn from_single_file(
    tag: impl ToString,
    path: impl AsRef<Path>,
) -> Result<(), MetaCallLoaderError> {
    from_file(tag, [path])
}
/// Loads a script from file. Usage example: ...
/// ```
/// // A Nodejs script
/// metacall::load::from_file("node", ["index.js", "main.js"]).unwrap();
/// ```
pub fn from_file(
    tag: impl ToString,
    paths: impl IntoIterator<Item = impl AsRef<Path>>,
) -> Result<(), MetaCallLoaderError> {
    let c_tag = cstring_enum!(tag, MetaCallLoaderError)?;
    let mut c_script: CString;

    let mut new_scripts: Vec<*const i8> = Vec::new();
    for path in paths.into_iter() {
        let script_as_pathbuf = PathBuf::from(path.as_ref());
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
/// metacall::load::from_memory("node", script).unwrap();
/// ```
pub fn from_memory(tag: impl ToString, script: impl ToString) -> Result<(), MetaCallLoaderError> {
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
