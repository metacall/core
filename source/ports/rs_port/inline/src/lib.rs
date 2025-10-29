use proc_macro::TokenStream;
use quote::quote;

macro_rules! gen_inline_macro {
    ($($name:ident => $tag:ident),*) => (
        $(
            #[proc_macro]
            pub fn $name(input: TokenStream) -> TokenStream {
                let token_stream_input = proc_macro2::TokenStream::from(input);
                let buffer = token_stream_input.to_string();

                let result = quote! {{
                    ::metacall::load::from_memory(::metacall::load::Tag::$tag, #buffer.to_string()).unwrap()
                }};

                result.into()
            }
        )*
    )
}

gen_inline_macro!(
    py => Python,
    node => NodeJS,
    ts => TypeScript,
    cs => CSharp,
    rb => Ruby,
    cob => Cobol,
    rpc => RPC,
    java => Java,
    wasm => Wasm
);
