use api::{self, PrimitiveMetacallProtocolTypes};

use std::os::raw::{c_int, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_initialize(
    loader_impl: *mut c_void,
    _config: *mut c_void,
) -> *mut c_void {
    let boxed_loader_lifecycle_state = Box::new(api::LoaderLifecycleState::new(Vec::new()));

    compiler::initialize();

    api::define_type(
        loader_impl,
        "i8",
        PrimitiveMetacallProtocolTypes::Char,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "i16",
        PrimitiveMetacallProtocolTypes::Short,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "i32",
        PrimitiveMetacallProtocolTypes::Int,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "i64",
        PrimitiveMetacallProtocolTypes::Long,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );

    // Register initialization
    api::loader_lifecycle_register(loader_impl);

    Box::into_raw(boxed_loader_lifecycle_state) as *mut c_void
}
