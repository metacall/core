use proc_macro::TokenStream;
use quote::quote;

macro_rules! gen_inline_macro {
    ($name:ident) => (
        #[proc_macro]
        pub fn $name(input: TokenStream) -> TokenStream {
            let token_stream_input = proc_macro2::TokenStream::from(input);
            let buffer = token_stream_input.to_string();

            let result = quote! {{
                ::metacall::load_from_memory("$name", #buffer.to_string()).unwrap()
            }};

            result.into()
        }
    )
}

gen_inline_macro!(py);
gen_inline_macro!(node);
gen_inline_macro!(ts);
gen_inline_macro!(cs);
gen_inline_macro!(rb);
gen_inline_macro!(cob);
gen_inline_macro!(rpc);
gen_inline_macro!(java);
gen_inline_macro!(wasm);
