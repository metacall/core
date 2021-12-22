extern crate proc_macro2;

use proc_macro::TokenStream;
use quote::quote;

#[proc_macro]
pub fn py(input: TokenStream) -> TokenStream {
    let token_stream_input = proc_macro2::TokenStream::from(input);

    let buffer = token_stream_input.to_string();

    let result = quote! {
        let size = #buffer.len();
        let tag = ::std::ffi::CString::new("py").expect("'py' conversion to C String failed").as_ptr() as *const ::std::primitive::i8;
        let buffer = ::std::ffi::CString::new(#buffer).expect("Python script conversion to C String failed").as_ptr() as *const ::std::primitive::i8;
        let handle = 0 as ::std::os::raw::c_int as *mut *mut ::std::ffi::c_void;

        println!("The code is:{}", #buffer);
        println!("The size is:{}", size);

        let load_result = unsafe { ::metacall::abi_interface::metacall_load_from_memory(tag, buffer, size, handle) };

        if load_result != 0 {
            panic!("MetaCall failed to load the python script. Result: {}", load_result);
        } else {
            println!("The python script loaded successfully.");
        }
    };

    result.into()
}
