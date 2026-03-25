use compiler::api::{self, PrimitiveMetacallProtocolTypes};

use std::os::raw::c_void;

#[no_mangle]
pub extern "C" fn rs_loader_impl_initialize(
    loader_impl: *mut c_void,
    _config: *mut c_void,
) -> *mut c_void {
    // Add current_dir to execution path to allow relative search path
    let search_paths = match std::env::current_dir() {
        Ok(dir) => vec![dir],
        Err(e) => {
            eprintln!(
                "rs_loader_impl_initialize: unable to get current dir: {}, using empty search paths",
                e
            );
            vec![]
        }
    };
    let boxed_loader_lifecycle_state = Box::new(api::LoaderLifecycleState::new(search_paths));

    compiler::initialize();

    api::define_type(
        loader_impl,
        "i8",
        PrimitiveMetacallProtocolTypes::Char,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "i16",
        PrimitiveMetacallProtocolTypes::Short,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "i32",
        PrimitiveMetacallProtocolTypes::Int,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "i64",
        PrimitiveMetacallProtocolTypes::Long,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "f32",
        PrimitiveMetacallProtocolTypes::Float,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "f64",
        PrimitiveMetacallProtocolTypes::Double,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "Ptr",
        PrimitiveMetacallProtocolTypes::Pointer,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "Array",
        PrimitiveMetacallProtocolTypes::Array,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "Map",
        PrimitiveMetacallProtocolTypes::Map,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "String",
        PrimitiveMetacallProtocolTypes::String,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );
    api::define_type(
        loader_impl,
        "Null",
        PrimitiveMetacallProtocolTypes::Null,
        std::ptr::null_mut::<c_void>(),
        std::ptr::null_mut::<c_void>(),
    );

    // Register initialization
    api::loader_lifecycle_register(loader_impl);

    Box::into_raw(boxed_loader_lifecycle_state) as *mut c_void
}
