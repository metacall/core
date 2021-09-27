use crate::{Path, PointerToCvoid};

#[no_mangle]
// There's nothing necessary to do at this step yet
pub extern "C" fn rs_loader_impl_load_from_memory(_loader_impl: PointerToCvoid, _paths: Path) {}