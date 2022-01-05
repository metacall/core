extern crate proc_macro2;

use proc_macro::TokenStream;
use quote::quote;

#[proc_macro]
pub fn py(input: TokenStream) -> TokenStream {
    let token_stream_input = proc_macro2::TokenStream::from(input);
    let buffer = token_stream_input.to_string();

    let result = quote! {
        if let Err(e) = ::metacall::load_from_memory("py", #buffer.to_string()) {
            panic!("{}", e);
        }
    };

    result.into()
}
