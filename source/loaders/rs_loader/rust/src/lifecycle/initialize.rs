use crate::c_void;

use bridge_abi::{self, PrimitiveMetacallProtocolTypes};

#[no_mangle]
pub extern "C" fn rs_loader_impl_initialize(
    loader_impl: *mut c_void,
    _config: *mut c_void,
) -> *mut c_void {
    let boxed_loader_lifecycle_state = Box::new(bridge_abi::LoaderLifecycleState::new(Vec::new()));

    bridge_abi::define_type(
        loader_impl,
        "i8",
        PrimitiveMetacallProtocolTypes::Char,
        0,
        0,
    );
    bridge_abi::define_type(
        loader_impl,
        "i16",
        PrimitiveMetacallProtocolTypes::Short,
        0,
        0,
    );
    bridge_abi::define_type(
        loader_impl,
        "i32",
        PrimitiveMetacallProtocolTypes::Int,
        0,
        0,
    );
    bridge_abi::define_type(
        loader_impl,
        "i64",
        PrimitiveMetacallProtocolTypes::Long,
        0,
        0,
    );

    Box::into_raw(boxed_loader_lifecycle_state) as *mut c_void
}
