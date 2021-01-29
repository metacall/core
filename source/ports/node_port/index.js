/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

const addon = (() => {
	try {
		/* This forces metacall port to be run always by metacall cli */
		return process.binding('node_loader_port_module');
	} catch (e) {
		console.error('MetaCall failed to load, probably you are importing this file from NodeJS directly.');
		console.error('You should use MetaCall CLI instead. Install it from: https://github.com/metacall/install');
		throw e;

		/* TODO: Until we find a better way to do this, we should disable it */
		/*
		const write = (data, cb) => {
			if (!process.stdout.write(data)) {
				process.stdout.once('drain', cb);
			} else {
				process.nextTick(cb);
			}
		};

		// Notify synchronously that we are launching MetaCall
		write('NodeJS detected, launching MetaCall...\n', () => {
			try {
				const { spawnSync } = require('child_process');
				const args = [...process.argv];

				args.shift();

				const result = spawnSync('metacall', args, {});

				if (result.error && result.error.code === 'ENOENT') {
					write('MetaCall not found. Please install MetaCall from: https://github.com/metacall/install and run it again.\n', () => {
						process.exit(1);
					});
				}

				process.exit(result.status !== null ? result.status : 1);
			} catch (e) {
				const message = 'MetaCall failed to load, probably you are importing this file from NodeJS directly.\n'
					+ e.message + '\n'
					+ 'Install MetaCall from: https://github.com/metacall/install and run it again.\n';

				write(message, () => {
					throw e;
				});
			}
		});
		*/
	}
})();

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

const metacall_load_from_memory = (tag, code) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(code) !== '[object String]') {
		throw Error('Code should be a string with the inline code to be loaded.');
	}

	return addon.metacall_load_from_memory(tag, code);
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

const metacall_require = (tag, name) => {
	// TODO: Inspect only the handle instead of the whole metacall namespace
	/* return */ addon.metacall_load_from_file(tag, [ name ]);

	const inspect = metacall_inspect();
	const script = inspect[tag].find(s => s.name === path.basename(name));

	const obj = {};

	for (const func of script.scope.funcs) {
		obj[func.name] = (...args) => addon.metacall(func.name, ...args);
	}

	return obj;
};

/* Module exports */
const module_exports = {
	metacall,
	metacall_inspect,
	metacall_load_from_file,
	metacall_load_from_memory,
	metacall_handle,

	/* TODO: Remove this from user or provide better ways of configuring logs */
	metacall_logs: () => {
		addon.metacall_logs();
	},
};

/* Monkey patch require for simplifying load */
const node_require = mod.prototype.require;

mod.prototype.require = function (name) {

	/* Extension -> Tag */
	const tags = {
		/* Mock Loader */
		mock: 'mock',
		/* Python Loader */
		py: 'py',
		/* Ruby Loader */
		rb: 'rb',
		/* C# Loader */
		cs: 'cs',
		vb: 'cs',
		dll: 'cs',
		/* Cobol Loader */
		cob: 'cob',
		cbl: 'cob',
		cpy: 'cob',
		/* NodeJS Loader */
		js: 'node',
		node: 'node',
		/* TypeScript Loader */
		ts: 'ts',
		jsx: 'ts',
		tsx: 'ts',

		/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
		/* Probably in the future we can differenciate between them, but it is not trivial */
	};

	/* Try to load it with NodeJS first */
	try {
		return node_require.apply(this, [ name ]);
	} catch (e) {
		if (e.code !== 'MODULE_NOT_FOUND') {
			throw e;
		}
	}

	try {
		return node_require.apply(this, [ require.resolve(name) ]);
	} catch (e) {
		if (e.code !== 'MODULE_NOT_FOUND') {
			throw e;
		}
	}

	const index = name.lastIndexOf('.');

	if (index !== -1) {
		/* If there is extension, load the module depending on the tag */
		const extension = name.substr(index + 1);
		const tag = tags[extension];

		if (tag && tag !== 'node') {
			/* Load with MetaCall if we found a tag and it is not NodeJS */
			return metacall_require(tag, name);
		}
	}

	/* If there is no extension or the extension is not supported or it is 'node', load it with NodeJS require */
	try {
		const filename = require.resolve(name);

		/* Cache the port (detect if this file is being loaded) */
		if (filename === path.resolve(__filename)) {
			return module_exports;
		}

		/* Call to real NodeJS require */
		return node_require.apply(this, [ filename ]);
	} catch (e) {
		/* If it is not a NodeJS module, try to guess the runtime */
		const loaders = new Set(Object.values(tags));

		/* Mock and node loaders are not included */
		loaders.delete('mock');
		loaders.delete('node');

		for (let it = loaders.values(), tag = null; tag = it.next().value; ) {
			try {
				return metacall_require(tag, name);
			} catch (_) {
				/* Keep trying with the next loader */
			}
		}

		/* It could not be loaded */
		console.log(e);
		throw e;
	}
};

/* Debug logs */
if (process.env['NODE_ENV'] === 'debug' && addon !== undefined) {
	addon.metacall_logs();
}

/* Export the API */
module.exports = module_exports;
