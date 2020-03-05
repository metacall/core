/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

'use strict';

const mod = require('module');
const path = require('path');
const addon = require(path.resolve(__dirname, './addon.js'));

const node_require = mod.prototype.require;

const metacall = (name, ...args) => {
	if (Object.prototype.toString.call(name) !== '[object String]') {
		throw Error('Function name should be of string type.');
	}

	return addon.metacall(name, ...args);
};

const metacall_load_from_file = (tag, paths) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (!(paths instanceof Array)) {
		throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
	}

	return addon.metacall_load_from_file(tag, paths);
};

const metacall_inspect = () => {
	const json_data = addon.metacall_inspect();

	if (json_data !== undefined) {
		const json = JSON.parse(json_data);

		delete json['__metacall_host__'];

		return json;
	}

	return {};
};

const metacall_handle = (tag, name) => {
	// TODO: This can be implemented with metacall_handle C API, meanwhile we use this trick
	const inspect = metacall_inspect();

	if (inspect === {} || inspect === undefined) {
		return null;
	}

	const ctx = inspect[tag];

	if (ctx === undefined) {
		return null;
	}

	return ctx.find(script => script.name === name);
};

const metacall_require = (tag, name, id) => {
	// TODO: Inspect only the handle instead of the whole metacall namespace
	/* return */ addon.metacall_load_from_file(tag, [ name ]);

	const inspect = metacall_inspect();
	const script = inspect[tag].find(script => script.name === id);
	const obj = {};

	for (const func of script.scope.funcs) {
		obj[func.name] = (...args) => addon.metacall(func.name, ...args);
	}

	return obj;
};

/* Monkey patch require for simplifying load */
mod.prototype.require = function (name) {

	const tags = {
		mock: 'mock',
		py: 'py',
		rb: 'rb',
		cs: 'cs',
		/*dll: 'cs',*/
	};

	const index = name.lastIndexOf('.');

	if (index === -1) {
		return node_require.apply(this, [ name ]);
	} else {
		// Load the module
		const extension = name.substr(index + 1);
		const id = path.basename(name.substr(0, index));
		const tag = tags[extension];

		if (tag) {
			return metacall_require(tag, name, id);
		} else {
			return node_require.apply(this, [ name ]);
		}
	}
};

/* Debug logs */
if (process.env['NODE_ENV'] === 'debug')
{
	addon.metacall_logs();
}

/* Export the API */
module.exports = {
	metacall,
	metacall_inspect,
	metacall_load_from_file,
	metacall_handle,

	/* TODO: Remove this from user or provide better ways of configuring logs */
	metacall_logs: () => {
		addon.metacall_logs();
	},
};
