use std::{
    env,
    path::{PathBuf},
};

use proc_macro::TokenStream;
use quote::quote;

macro_rules! gen_inline_macro {
    ($($name:ident),*) => (
        $(
            #[proc_macro]
            pub fn $name(input: TokenStream) -> TokenStream {
                let token_stream_input = proc_macro2::TokenStream::from(input);
                let buffer = token_stream_input.to_string();

                let result = quote! {{
                    ::metacall::loaders::from_memory(stringify!($name), #buffer.to_string()).unwrap()
                }};

                result.into()
            }
        )*
    )
}

gen_inline_macro!(py, node, ts, cs, rb, cob, rpc, java, wasm);


#[proc_macro]
pub fn include_bindings(_input: TokenStream) -> TokenStream {
    let out_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let path_dir = out_dir.join("src").join("bindings").join("bindings.rs");
    let path = path_dir.to_str();

    let result = quote! {
        #[path = #path]
        #[allow(warnings)]
        pub mod bindings;
    };

    result.into()
}
