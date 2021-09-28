use crate::{c_int, PointerToCvoid};

#[no_mangle]
// There's nothing necessary to do at this step yet
pub extern "C" fn rs_loader_impl_destroy(_loader_impl: PointerToCvoid) -> c_int {
    0 as c_int
}
