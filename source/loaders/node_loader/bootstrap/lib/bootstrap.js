#!/usr/bin/env node
'use strict';

// eslint-disable-next-line global-require
const trampoline = require('./trampoline.node');

const Module = require('module');
const path = require('path');
const util = require('util');

const cherow = require('./node_modules/cherow');

function node_loader_trampoline_is_callable(value) {
	return typeof value === 'function';
}

function node_loader_trampoline_is_valid_symbol(node) {
	// TODO: Enable more function types
	return node.type === 'FunctionExpression' || node.type === 'ArrowFunctionExpression';
}

function node_loader_trampoline_module(m) {
	if (node_loader_trampoline_is_callable(m)) {
		const wrapper = {};

		wrapper[m.name] = m;

		return wrapper;
	}

	return m;
};

// eslint-disable-next-line no-empty-function
function node_loader_trampoline_execution_path() {
	// TODO
}

function node_loader_trampoline_load_from_file(paths) {
	if (!Array.isArray(paths)) {
		throw new Error('Load from file paths must be an array, not ' + typeof paths);
	}

	try {
		const handle = {};

		for (let i = 0; i < paths.length; ++i) {
			const p = paths[i];
			const m = require(path.resolve(__dirname, p));

			handle[p] = node_loader_trampoline_module(m);
		}

		return handle;
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_load_from_file', ex);
	}

	return null;
}

function node_loader_trampoline_load_from_memory(name, buffer, opts) {
	if (typeof name !== 'string') {
		throw new Error('Load from memory name must be a string, not ' + typeof name);
	}

	if (typeof buffer !== 'string') {
		throw new Error('Load from memory buffer must be a string, not ' + typeof buffer);
	}

	if (typeof opts !== 'object') {
		throw new Error('Load from memory opts must be an object, not ' + typeof opts);
	}

	const paths = Module._nodeModulePaths(path.dirname(name));
	const parent = module.parent;
	const m = new Module(name, parent);

	m.filename = name;
	m.paths = [
		...opts.prepend_paths || [],
		...paths,
		...opts.append_paths || [],
	];

	// eslint-disable-next-line no-underscore-dangle
	m._compile(buffer, name);

	if (parent && parent.children) {
		parent.children.splice(parent.children.indexOf(m), 1);
	}

	const handle = {};

	handle[name] = node_loader_trampoline_module(m.exports);

	return handle;
}

// eslint-disable-next-line no-empty-function
function node_loader_trampoline_load_from_package() {
	// TODO
}

function node_loader_trampoline_clear(handle) {
	try {
		const names = Object.getOwnPropertyNames(handle);

		for (let i = 0; i < names.length; ++i) {
			const p = names[i];
			const absolute = path.resolve(__dirname, p);

			if (require.cache[absolute]) {
				delete require.cache[absolute];
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_clear', ex);
	}
}

function node_loader_trampoline_discover_arguments_generate(args, index) {
	let name = `_arg${index}`;

	while (args.includes(name)) {
		name = `_${name}`;
	}

	return name;
}

function node_loader_trampoline_discover_arguments(node) {
	const params = node.params;
	const args = [];

	for (let i = 0; i < params.length; ++i) {
		const p = params[i];

		if (p.type === 'Identifier') {
			args.push(p.name);
		} else if (p.type === 'AssignmentPattern' && p.left && p.left.type === 'Identifier') {
			args.push(p.left.name);
		} else if (p.type === 'ObjectPattern') {
			// TODO: Use this trick until meta object protocol
			args.push(null);
		}
	}

	// TODO: Use this trick until meta object protocol
	for (let i = 0; i < params.length; ++i) {
		const p = args[i];

		if (p === null) {
			args[i] = node_loader_trampoline_discover_arguments_generate(args, i);
		}
	}

	return args;
}

function node_loader_trampoline_discover(handle) {
	const discover = {};

	try {
		const names = Object.getOwnPropertyNames(handle);

		for (let i = 0; i < names.length; ++i) {
			const exports = handle[names[i]];
			const keys = Object.getOwnPropertyNames(exports);

			for (let j = 0; j < keys.length; ++j) {
				const key = keys[j];
				const func = exports[key];

				if (node_loader_trampoline_is_callable(func)) {
					const ast = cherow.parse(`(${func.toString()})`, {
						module: false,
						skipShebang: true,
					}).body[0];

					const node = ast.expression;

					if (node_loader_trampoline_is_valid_symbol(node)) {
						const args = node_loader_trampoline_discover_arguments(node);

						discover[key] = {
							ptr: func,
							signature: args,
						};
					}
				}
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_discover', ex);
	}

	return discover;
}

function node_loader_trampoline_test(obj) {
	console.log('NodeJS Loader Bootstrap Test');

	if (obj !== undefined) {
		console.log(util.inspect(obj, false, null, true));
	}
}

function node_loader_trampoline_await(func, args, trampoline_ptr) {
	if (typeof func !== 'function') {
		throw new Error('Await only accepts a callable function func, not ' + typeof func);
	}

	if (!Array.isArray(args)) {
		throw new Error('Await only accepts a array as arguments args, not ' + typeof args);
	}

	if (typeof trampoline_ptr !== 'object') {
		throw new Error('Await trampoline_ptr must be an object, not ' + typeof trampoline_ptr);
	}

	return new Promise((resolve, reject) =>
		func(...args).then(
			x => resolve(trampoline.resolve(trampoline_ptr, x)),
			x => reject(trampoline.reject(trampoline_ptr, x)),
		).catch(
			x => console.error(`NodeJS await error: ${x && x.message ? x.message : util.inspect(x, false, null, true)}`),
		)
	);
}

function node_loader_trampoline_destroy() {
	try {
		// eslint-disable-next-line no-underscore-dangle
		const handles = process._getActiveHandles();

		for (let i = 0; i < handles.length; ++i) {
			const h = handles[i];

			// eslint-disable-next-line no-param-reassign, no-empty-function
			h.write = function () {};
			// eslint-disable-next-line max-len
			// eslint-disable-next-line no-param-reassign, no-underscore-dangle, no-empty-function
			h._destroy = function () {};

			if (h.end) {
				h.end();
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_destroy', ex);
	}
}

module.exports = ((impl, ptr) => {
	return trampoline.register(impl, ptr, {
		'execution_path': node_loader_trampoline_execution_path,
		'load_from_file': node_loader_trampoline_load_from_file,
		'load_from_memory': node_loader_trampoline_load_from_memory,
		'load_from_package': node_loader_trampoline_load_from_package,
		'clear': node_loader_trampoline_clear,
		'discover': node_loader_trampoline_discover,
		'test': node_loader_trampoline_test,
		'await': node_loader_trampoline_await,
		'destroy': node_loader_trampoline_destroy,
	});
})(process.argv[2], process.argv[3]);
