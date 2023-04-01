/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

use metacall_inline;
pub mod inline {
    pub use metacall_inline::*;
}

metacall_inline::include_bindings!();
pub(crate) use generated_bindings as bindings;
pub(crate) mod macros;
pub(crate) mod parsers;

pub mod hooks;
pub mod loaders;
pub mod prelude;

#[path = "metacall.rs"]
mod metacall_mod;
pub use metacall_mod::metacall;
