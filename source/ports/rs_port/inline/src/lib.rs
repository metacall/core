extern crate proc_macro2;

use proc_macro::TokenStream;
use quote::quote;

// include generated macros
include!(concat!(env!("OUT_DIR"), "/inline_gen.rs"));
