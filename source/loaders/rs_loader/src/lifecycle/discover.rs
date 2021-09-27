use crate::{PointerToCvoid};

#[no_mangle]
// There's nothing necessary to do at this step yet
pub extern "C" fn rs_loader_impl_discover(_loader_impl: PointerToCvoid) {}