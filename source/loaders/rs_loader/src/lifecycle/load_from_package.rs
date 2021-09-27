use crate::{Path, PointerToCvoid};

#[no_mangle]
// There's nothing necessary to do at this step yet
pub extern "C" fn rs_loader_impl_load_from_package(_loader_impl: PointerToCvoid, _path: Path) {}