use std::env;
use std::fs::File;
use std::io::Write;
use std::path::Path;

fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("inline_gen.rs");
    let mut f = File::create(&dest_path).unwrap();

    let loaders = vec!["py", "node", "ts", "cs", "rb", "cob", "rpc", "java", "wasm"];
    for name in loaders {
        let content = format!("
#[proc_macro]
pub fn {name}(input: TokenStream) -> TokenStream {{
    let token_stream_input = proc_macro2::TokenStream::from(input);
    let buffer = token_stream_input.to_string();

    let result = quote! {{
        if let Err(e) = ::metacall::load_from_memory(\"{name}\", #buffer.to_string()) {{
            panic!(\"{{}}\", e);
        }}
    }};

    result.into()
}}
        ");

        f.write_all(content.as_bytes()).unwrap();
    }
}