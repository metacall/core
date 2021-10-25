use super::loader::{self, LoadingMethod};
use crate::{c_char, c_void, PathBuf};

use metacall_registrator::file::FileRegistration;

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_file(
    loader_impl: *mut c_void,
    paths: *mut *const c_char,
    size: usize,
) -> *mut c_void {
    loader::load(
        loader_impl,
        paths,
        size,
        true,
        |path_buf: PathBuf, _| -> Result<LoadingMethod, *mut c_void> {
            Ok(LoadingMethod::File(FileRegistration::new(path_buf)))
        },
    )
}
