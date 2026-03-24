#!/usr/bin/env node
/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* Regression test for MetaCall NAPI native addon support.
 * @rspack/core depends on @rspack/binding, a Rust-compiled NAPI addon (napi-rs).
 * On Windows, MetaCall must resolve napi_* symbols correctly for native addons.
 * This covers the rspack regression reproduced by distributable-windows.
 */
const { rspackVersion } = require('@rspack/core');

module.exports = {
	check: function check() {
		console.log('rspack loaded, version: ' + rspackVersion);
	},
};
