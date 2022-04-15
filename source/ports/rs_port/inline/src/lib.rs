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
                    ::metacall::load_from_memory(stringify!($name), #buffer.to_string()).unwrap()
                }};

                result.into()
            }
        )*
    )
}

gen_inline_macro!(py, node, ts, cs, rb, cob, rpc, java, wasm);
