use crate::{
    bindings::{metacall_load_from_file, metacall_load_from_memory},
    cstring_enum,
    prelude::MetacallLoaderError,
};
use std::{
    ffi::CString,
    path::{Path, PathBuf},
    ptr,
};

pub fn from_file(
    tag: impl ToString,
    scripts: impl IntoIterator<Item = impl AsRef<Path>>,
) -> Result<(), MetacallLoaderError> {
    let c_tag = cstring_enum!(tag, MetacallLoaderError)?;
    let mut c_script: CString;

    let mut new_scripts: Vec<*const i8> = Vec::new();
    for script in scripts.into_iter() {
        let script_as_pathbuf = PathBuf::from(script.as_ref());
        let script_as_str = script_as_pathbuf.to_str().unwrap();

        if !script_as_pathbuf.exists() {
            return Err(MetacallLoaderError::FileNotFound(script_as_pathbuf));
        }
        if !script_as_pathbuf.is_file() {
            return Err(MetacallLoaderError::NotAFileOrPermissionDenied(
                script_as_pathbuf,
            ));
        }

        c_script = cstring_enum!(script_as_str, MetacallLoaderError)?;

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
        return Err(MetacallLoaderError::FromFileFailure);
    }

    Ok(())
}

pub fn from_memory(tag: impl ToString, script: impl ToString) -> Result<(), MetacallLoaderError> {
    let script = script.to_string();
    let c_tag = cstring_enum!(tag, MetacallLoaderError)?;
    let c_script = cstring_enum!(script, MetacallLoaderError)?;

    if unsafe {
        metacall_load_from_memory(
            c_tag.as_ptr(),
            c_script.as_ptr(),
            script.len(),
            ptr::null_mut(),
        )
    } != 0
    {
        return Err(MetacallLoaderError::FromMemoryFailure);
    }

    Ok(())
}
