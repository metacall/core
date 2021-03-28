#!/usr/bin/env node
'use strict';

const Module = require('module');
const path = require('path');
const util = require('util');
const fs = require('fs');
const async_hooks = require('async_hooks');

/* Require the JavaScript parser */
const cherow = require(path.join(__dirname, 'node_modules', 'cherow'));

const node_require = Module.prototype.require;
const node_resolve = require.resolve;
const node_cache = require.cache;

/* Store in the module prototype the original functions for future use in derived loaders like TypeScript */
Module.prototype.node_require = node_require;
Module.prototype.node_resolve = node_resolve;
Module.prototype.node_cache = node_cache;

function node_loader_trampoline_initialize() {
	const global_path = process.env['LOADER_LIBRARY_PATH'];

	const paths = [
		/* Local version of MetaCall NodeJS Port */
		'metacall',
		/* Optionally, use loader library path for global installed NodeJS Port */
		...global_path ? [ path.join(global_path, 'node_modules', 'metacall', 'index.js') ] : [],
	];

	for (const r of paths) {
		try {
				return node_require(r);
		} catch (e) {
			if (e.code !== 'MODULE_NOT_FOUND') {
				console.log(`NodeJS Error (while preloading MetaCall): ${e.message}`);
			}
		}
	}

	process.env.NODE_PATH += global_path;

	console.log('NodeJS Warning: MetaCall could not be preloaded');
}

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

function node_loader_trampoline_load_from_file_require(p) {
	const basename = path.basename(p);
	const { name } = path.parse(basename);

	const paths = [
		/* Absolute path or module name */
		p,
		/* Without base path */
		path.basename(p),
		/* Without base path and extension */
		name,
		/* Without extension and with node modules */
		path.join(path.dirname(p), 'node_modules', name),
	];

	for (const r of paths) {
		let resolved = null;

		try {
			resolved = node_resolve(r);
		} catch (e) {
			if (e.code !== 'MODULE_NOT_FOUND') {
				throw e;
			}
		}

		if (resolved != null) {
			return node_require(resolved);
		}
	}

	const e = new Error(`Cannot find module '${p}'`);
	e.code = 'MODULE_NOT_FOUND';
	throw e;
}

function node_loader_trampoline_load_from_file(paths) {
	if (!Array.isArray(paths)) {
		throw new Error('Load from file paths must be an array, not ' + typeof paths);
	}

	try {
		const handle = {};

		for (let i = 0; i < paths.length; ++i) {
			const p = paths[i];
			const m = node_loader_trampoline_load_from_file_require(path.resolve(__dirname, p));

			handle[p] = node_loader_trampoline_module(m);
		}

		return handle;
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_load_from_file while loading:', paths, ex);
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

	try {
		// eslint-disable-next-line no-underscore-dangle
		m._compile(buffer, name);
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_load_from_memory while loading:', buffer, ex);
		return null;
	}

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

			if (node_cache[absolute]) {
				delete node_cache[absolute];
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

function node_loader_trampoline_discover_function(func) {
	try {
		if (node_loader_trampoline_is_callable(func)) {
			// Cherow can't parse native code functions so we can do a workaround
			const str = func.toString().replace('{ [native code] }', '{}');

			const ast = cherow.parse(`(${str})`, {
				module: false,
				skipShebang: true,
			}).body[0];

			const node = ast.expression;

			if (node_loader_trampoline_is_valid_symbol(node)) {
				const args = node_loader_trampoline_discover_arguments(node);

				const discover = {
					ptr: func,
					signature: args,
					async: node.async,
				};

				if (node.id && node.id.name) {
					discover['name'] = node.id.name;
				}

				return discover;
			}
		}
	} catch (ex) {
		console.log(`Exception while parsing '${func}' in node_loader_trampoline_discover_function`, ex);
	}
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
				const descriptor = node_loader_trampoline_discover_function(func);

				if (descriptor !== undefined) {
					discover[key] = descriptor;
				}
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_discover', ex);
	}

	return discover;
}

function node_loader_trampoline_test(obj) {
	/* Imporant: never trigger an async resource in this function */
	if (obj !== undefined) {
		fs.writeSync(process.stdout.fd, `${util.inspect(obj, false, null, true)}\n`);
	}
}

function node_loader_trampoline_await_function(trampoline) {
	if (!trampoline) {
		return function node_loader_trampoline_await_impl(func, args, trampoline_ptr) {
			console.error('NodeJS Loader await error, trampoline could not be found, await calls are disabled.');
		};
	}

	return function node_loader_trampoline_await_impl(func, args, trampoline_ptr) {
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
				x => reject(trampoline.reject(trampoline_ptr, x))
			).catch(
				x => console.error(`NodeJS await error: ${x && x.message ? x.message : util.inspect(x, false, null, true)}`)
			)
		);
	};
}

function node_loader_trampoline_await_future(trampoline) {
	if (!trampoline) {
		return function node_loader_trampoline_await_impl(func, args, trampoline_ptr) {
			console.error('NodeJS Loader await error, trampoline could not be found, await calls are disabled.');
		};
	}

	return function node_loader_trampoline_await_impl(future, trampoline_ptr) {
		if (!(!!future && typeof future.then === 'function')) {
			throw new Error('Await only accepts a thenable promise, not ' + typeof future);
		}

		if (typeof trampoline_ptr !== 'object') {
			throw new Error('Await trampoline_ptr must be an object, not ' + typeof trampoline_ptr);
		}

		return new Promise((resolve, reject) =>
			future.then(
				x => resolve(trampoline.resolve(trampoline_ptr, x)),
				x => reject(trampoline.reject(trampoline_ptr, x))
			).catch(
				x => console.error(`NodeJS await error: ${x && x.message ? x.message : util.inspect(x, false, null, true)}`)
			)
		);
	};
}

module.exports = ((impl, ptr) => {
	try {
		if (typeof impl === 'undefined' || typeof ptr === 'undefined') {
			throw new Error('Process arguments (process.argv[2], process.argv[3]) not defined.');
		}

		/* Initialize async hooks for keeping track the amount of async handles that are in the event queue */
		const asyncHook = async_hooks.createHook({
			init: node_loader_trampoline_async_hook_init,
			destroy: node_loader_trampoline_async_hook_destroy,
		});

		/* Map for tracking all the async references */
		const asyncMap = new Map();

		/* Get trampoline from list of linked bindings */
		const trampoline = process._linkedBinding('node_loader_trampoline_module');

		function node_loader_trampoline_async_count(cleanHooks) {
			/* Count the number of active resources, taking into account the reference timeouts */
			const activeResources = [...asyncMap.values()].filter(
				(asyncRes) =>
					!(asyncRes.type === 'Timeout'
					&& typeof asyncRes.resource.hasRef === 'function'
					&& !asyncRes.resource.hasRef())
			);

			if (cleanHooks === true && activeResources.length === 0) {
				/* Disable Hooks */
				asyncHook.disable();
			}

			return activeResources.length;
		}

		const node_loader_ptr = trampoline.register(impl, ptr, {
			'initialize': node_loader_trampoline_initialize,
			'execution_path': node_loader_trampoline_execution_path,
			'load_from_file': node_loader_trampoline_load_from_file,
			'load_from_memory': node_loader_trampoline_load_from_memory,
			'load_from_package': node_loader_trampoline_load_from_package,
			'clear': node_loader_trampoline_clear,
			'discover': node_loader_trampoline_discover,
			'discover_function': node_loader_trampoline_discover_function,
			'test': node_loader_trampoline_test,
			'await_function': node_loader_trampoline_await_function(trampoline),
			'await_future': node_loader_trampoline_await_future(trampoline),
			'async_count': () => node_loader_trampoline_async_count(true),
		});

		/* Enable async hook with the counter started to 0 */
		asyncHook.enable();

		function node_loader_trampoline_async_hook_init(asyncId, type, triggerAsyncId, resource) {
			/* For some reason I do not know, NodeJS does not count this handles as async handles
			* for closing the event loop, so we must avoid them */
			if (type !== 'TIMERWRAP' && type !== 'PROMISE') {
				asyncMap.set(asyncId, {type, triggerAsyncId, resource});
			}
		}

		function node_loader_trampoline_async_hook_destroy(asyncId) {
			if (asyncMap.has(asyncId)) {
				/* Clean up async resource from the map */
				asyncMap.delete(asyncId);

				/* At this point we may have reached an "empty" event loop, it
				* only contains the async resources that the Node Loader has populated,
				* but it does not contain any user defined async resource */
				if (node_loader_trampoline_async_count(false) === 0 && trampoline.requested_destroy(node_loader_ptr) === true) {
					/* Disable Hooks and destroy the Node Loader */
					asyncHook.disable();
					trampoline.destroy(node_loader_ptr);
				}
			}
		}
	} catch (ex) {
		console.log('Exception in bootstrap.js trampoline initialization:', ex);
	}
})(process.argv[2], process.argv[3]);
