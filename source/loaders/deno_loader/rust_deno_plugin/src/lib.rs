use deno_core::plugin_api::*;

#[no_mangle]
pub fn deno_plugin_init(interface: &mut dyn Interface) {
    interface.register_op("metacall", metacall);
}

fn metacall(_interface: &mut dyn Interface, zero_copy: &mut [ZeroCopyBuf]) -> Op {
    Op::Sync(Box::new(*b"meme"))
}
