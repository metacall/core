use compiler::api::{self, PrimitiveMetacallProtocolTypes};

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
    // api::define_type(
    //     loader_impl,
    //     "Usize",
    //     PrimitiveMetacallProtocolTypes::Int,
    //     0 as c_int as *mut c_void,
    //     0 as c_int as *mut c_void,
    // );
    // api::define_type(
    //     loader_impl,
    //     "U8",
    //     PrimitiveMetacallProtocolTypes::Char,
    //     0 as c_int as *mut c_void,
    //     0 as c_int as *mut c_void,
    // );
    // api::define_type(
    //     loader_impl,
    //     "U16",
    //     PrimitiveMetacallProtocolTypes::Short,
    //     0 as c_int as *mut c_void,
    //     0 as c_int as *mut c_void,
    // );
    // api::define_type(
    //     loader_impl,
    //     "U32",
    //     PrimitiveMetacallProtocolTypes::Int,
    //     0 as c_int as *mut c_void,
    //     0 as c_int as *mut c_void,
    // );
    // api::define_type(
    //     loader_impl,
    //     "U64",
    //     PrimitiveMetacallProtocolTypes::Long,
    //     0 as c_int as *mut c_void,
    //     0 as c_int as *mut c_void,
    // );
    api::define_type(
        loader_impl,
        "f32",
        PrimitiveMetacallProtocolTypes::Float,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "f64",
        PrimitiveMetacallProtocolTypes::Double,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "Ptr",
        PrimitiveMetacallProtocolTypes::Pointer,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "Array",
        PrimitiveMetacallProtocolTypes::Array,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "Map",
        PrimitiveMetacallProtocolTypes::Map,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "String",
        PrimitiveMetacallProtocolTypes::String,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    api::define_type(
        loader_impl,
        "Null",
        PrimitiveMetacallProtocolTypes::Null,
        0 as c_int as *mut c_void,
        0 as c_int as *mut c_void,
    );
    // Register initialization
    api::loader_lifecycle_register(loader_impl);

    Box::into_raw(boxed_loader_lifecycle_state) as *mut c_void
}
