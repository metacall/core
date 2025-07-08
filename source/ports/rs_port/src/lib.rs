#![warn(clippy::all)]
#![allow(
    clippy::not_unsafe_ptr_arg_deref,
    clippy::boxed_local,
    clippy::tabs_in_doc_comments,
    clippy::needless_doctest_main
)]
/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

//! [METACALL](https://github.com/metacall/core) is a library that allows calling functions,
//! methods or procedures between programming languages. With METACALL you can transparently
//! execute code from / to any programming language, for example, call TypeScript code from Rust.
//! Click [here](https://github.com/metacall/install) for installation guide.
//!
//! General usage example:
//! Let's consider we have the following Typescript code:
//! `sum.ts`
//! ``` javascript
//! export function sum(a: number, b: number): number {
//!	    return a + b;
//! }
//! ```
//! Now let's jump into Rust:
//!
//! ```
//! use metacall::{switch, metacall, loaders};
//!
//! fn main() {
//!     // Initialize MetaCall at the top
//!     let _metacall = switch::initialize().unwrap();
//!     
//!     // Load the file (Checkout the loaders module for loading multiple files
//!     // or loading from string)
//!     load::from_single_file("ts", "sum.ts").unwrap();
//!
//!     // Call the sum function (Also checkout other metacall functions)
//!     let sum = metacall::<f64>("sum", [1.0, 2.0]).unwrap();
//!
//!     assert_eq!(sum, 3.0);
//! }
//!
//! ```

pub(crate) mod cast;
pub(crate) mod helpers;
pub(crate) use macros::private_macros::*;

/// Contains MetaCall loaders from file and memory. Usage example: ...
/// ```
/// // Loading a single file with Nodejs.
/// metacall::load::from_single_file("node", "index.js").unwrap();
///
/// // Loading multiple files with Nodejs.
/// metacall::load::from_file("node", ["index.js", "main.js"]).unwrap();
///
/// // Loading a string with Nodejs.
/// let script = "function greet() { return 'hi there!' }; module.exports = { greet };";
/// metacall::load::from_memory("node", script).unwrap();
/// ```
pub mod load;

mod types;

#[doc(hidden)]
pub mod macros;

#[doc(hidden)]
pub use types::*;

#[doc(hidden)]
mod init;

pub use cast::metacall_box;
pub use init::initialize;
pub use init::is_initialized;

#[path = "metacall.rs"]
mod metacall_mod;
pub use metacall_mod::*;

/// Contains MetaCall language inliners. Usage example: ...
/// ```
/// // Python
/// py! {
///     print("hello world")
/// }
///
/// // Nodejs
/// node! {
///     console.log("hello world");
/// }
///
/// // Typescript
/// ts! {
///     console.log("hello world");
/// }
/// ```
pub mod inline {
    pub use metacall_inline::*;
}

#[allow(warnings)]
#[doc(hidden)]
pub mod bindings;
