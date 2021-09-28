use crate::{c_int, PointerToCvoid};

#[no_mangle]
// There's nothing necessary to do at this step yet
pub extern "C" fn rs_loader_impl_initialize(_loader_impl: PointerToCvoid, _config: PointerToCvoid) -> PointerToCvoid {
    1 as *const c_int as PointerToCvoid
}
