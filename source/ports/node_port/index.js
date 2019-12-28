'use strict';

const Module = require('module');
const Addon = require('./addon');

const node_require = Module.prototype.require;

const metacall_require = (tag, name) => {
	// TODO: Inspect the current handle and append it to an object mocking the function calls with metacall
	return Addon.metacall_load_from_file(tag, [ name ]);
};

/* Monkey patch require for simplifying load */
Module.prototype.require = function (id) {

	const tags = {
		mock: 'mock',
		py: 'py',
		rb: 'rb',
		cs: 'cs',
		/*dll: 'cs',*/
	};

	const index = id.lastIndexOf('.');

	if (index === -1) {
		return node_require.apply(this, [ id ]);
	} else {
		// Load the module
		const extension = id.substr(index + 1);
		const tag = tags[extension];

		if (tag) {
			return metacall_require(tag, id);
		} else {
			return node_require.apply(this, [ id ]);
		}
	}
};

/* Debug logs */
if (process.env['NODE_ENV'] === 'debug')
{
	Addon.metacall_logs();
}

/* Export the API */
module.exports = {
	metacall: (name, ...args) => {
		if (Object.prototype.toString.call(name) !== '[object String]') {
			throw Error('Function name should be of string type.');
		}

		return Addon.metacall(name, ...args);
	},

	metacall_load_from_file: (tag, paths) => {
		if (Object.prototype.toString.call(tag) !== '[object String]') {
			throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
		}

		if (!(paths instanceof Array)) {
			throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
		}

		return Addon.metacall_load_from_file(tag, paths);
	},

	metacall_inspect: () => {
		const json_data = Addon.metacall_inspect();

		if (json_data !== undefined) {
			const json = JSON.parse(json_data);

			delete json['__metacall_host__'];

			return json;
		}
	},

	/* TODO: Remove this from user or provide better ways of configuring logs */
	metacall_logs: () => {
		Addon.metacall_logs();
	},
};
