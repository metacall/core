#!/usr/bin/env node
'use strict';

const Module = require('module');
const path = require('path');
const util = require('util');
const fs = require('fs');

// Require the JavaScript parser
const espree = require(path.join(__dirname, 'node_modules', 'espree'));

const node_require = Module.prototype.require;
const node_resolve = require.resolve;
const node_cache = require.cache;

// Store in the module prototype the original functions for future use in derived loaders like TypeScript
Module.prototype.node_require = node_require;
Module.prototype.node_resolve = node_resolve;
Module.prototype.node_cache = node_cache;

function node_loader_trampoline_initialize(loader_library_path) {
	// Restore the argv (this is used for tricking node::Start method)
	process.argv = [ process.argv[0] ];

	// Add current execution directory to the execution paths
	node_loader_trampoline_execution_path(process.cwd());

	const paths = [
		// Local version of MetaCall NodeJS Port
		'metacall',
		// Optionally, use loader library path for global installed NodeJS Port
		...loader_library_path ? [ path.join(loader_library_path, 'node_modules', 'metacall', 'index.js') ] : [],
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

	// console.log('NodeJS Warning: MetaCall could not be preloaded');
}

function node_loader_trampoline_is_callable(value) {
	return typeof value === 'function';
}

function node_loader_trampoline_is_valid_symbol(node) {
	return node.type === 'FunctionDeclaration'
		|| node.type === 'FunctionExpression'
		|| node.type === 'ArrowFunctionExpression'
		|| node.type === 'ClassDeclaration'
		|| node.type === 'ClassExpression';
}

function node_loader_trampoline_is_class(value) {
	if (typeof value !== 'function') {
		return false;
	}

	try {
		const str = value.toString();
		// ES6 classes start with "class " when converted to string
		if (str.startsWith('class ') || str.startsWith('class{')) {
			return true;
		}
	} catch (e) {
		// Some native functions may throw
	}

	return false;
}

function node_loader_trampoline_module(m) {
	if (node_loader_trampoline_is_callable(m)) {
		const wrapper = {};

		wrapper[m.name] = m;

		return wrapper;
	}

	return m;
};

function node_loader_trampoline_node_path() {
	// Retrieve NODE_PATH enviroment variable and convert it to array of strings
	return (process.env['NODE_PATH'] || '').split(path.delimiter).filter((e) => e.length !== 0);
}

function node_loader_trampoline_execution_path_push_unique(paths, p) {
	// Check if the path exists in its cannonical form (/a/b and /a/b/ are the same path)
	if (!paths.some(cur => path.relative(cur, p) === '')) {
		// Insert execution path
		paths.push(p);

		return true;
	}

	return false;
}

function node_loader_trampoline_execution_path(p) {
	if (p) {
		const paths = node_loader_trampoline_node_path();

		// Insert first the execution path as it is
		let moduleRefresh = node_loader_trampoline_execution_path_push_unique(paths, p);

		// Each added execution path will follow the NodeJS module style
		const modulePaths = Module._nodeModulePaths(p);

		for (const modulePath of modulePaths) {
			moduleRefresh = moduleRefresh || node_loader_trampoline_execution_path_push_unique(paths, modulePath);
		}

		if (moduleRefresh) {
			// Set the NODE_PATH environment variable again
			process.env['NODE_PATH'] = paths.join(path.delimiter);

			// Refresh the paths and make effective the changes
			Module._initPaths();
		}
	}
}

function node_loader_trampoline_import(method, p) {
	try {
		return method(p);
	} catch (ex) {
		if (ex.code !== 'MODULE_NOT_FOUND') {
			throw ex;
		}

		// Try global paths
		const paths = node_loader_trampoline_node_path();

		for (const r of paths) {
			try {
				return method(path.join(r, p));
			} catch (e) {
				if (e.code !== 'MODULE_NOT_FOUND') {
					throw e;
				}
			}
		}

		throw ex;
	}
}

function node_loader_trampoline_load_from_file(paths) {
	if (!Array.isArray(paths)) {
		throw new Error('Load from file paths must be an array, not ' + typeof paths);
	}

	try {
		const handle = {};

		for (let i = 0; i < paths.length; ++i) {
			const p = paths[i];
			const m = node_loader_trampoline_import(node_require, p);

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
			try {
				const absolute = node_loader_trampoline_import(node_resolve, p);

				if (node_cache[absolute]) {
					delete node_cache[absolute];
				}
			} catch (_) {}
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
			// Espree can't parse native code functions so we can do a workaround
			const str = func.toString().replace('{ [native code] }', '{}');

			const ast = (() => {
				try {
					// Parse the function
					return espree.parse(`(${str})`, {
						ecmaVersion: 14
					});
				} catch (e) {
					// Espree can't parse a function exported directly from a map without key, like:
					// const exports = { myfunc() {} };
					// Because it lacks from function prefix, so as a workaround, we add function prefix
					// if it does not exist. It is a nasty trick but it's the best solution I found by now.
					return espree.parse(`(function ${str})`, {
						ecmaVersion: 14
					});
				}
			})();

			const node = (ast.body[0].type === 'ExpressionStatement') ?
				ast.body[0].expression : ast.body[0];

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

function node_loader_trampoline_discover_klass_constructors(classNode) {
	const constructors = [];

	if (!classNode.body || !classNode.body.body) {
		// Default constructor
		constructors.push({
			name: 'constructor',
			signature: [],
			paramCount: 0
		});
		return constructors;
	}

	const constructor = classNode.body.body.find(
		m => m.type === 'MethodDefinition' && m.kind === 'constructor'
	);

	if (constructor) {
		const params = node_loader_trampoline_discover_arguments(constructor.value);
		constructors.push({
			name: 'constructor',
			signature: params,
			paramCount: params.length
		});
	} else {
		// Default constructor
		constructors.push({
			name: 'constructor',
			signature: [],
			paramCount: 0
		});
	}

	return constructors;
}

function node_loader_trampoline_discover_klass_methods(classNode, klass) {
	const methods = {};

	if (!classNode.body || !classNode.body.body) {
		return methods;
	}

	for (const member of classNode.body.body) {
		// Handle MethodDefinition nodes
		if (member.type === 'MethodDefinition') {
			// Skip constructor - handled separately
			if (member.kind === 'constructor') {
				continue;
			}

			const methodName = member.key.type === 'Identifier'
				? member.key.name
				: (member.key.value || String(member.key.name));

			const funcNode = member.value;
			const params = node_loader_trampoline_discover_arguments(funcNode);

			// Get the actual function reference
			let ptr = null;
			if (member.static) {
				ptr = klass[methodName];
			} else {
				ptr = klass.prototype ? klass.prototype[methodName] : null;
			}

			methods[methodName] = {
				ptr: ptr,
				name: methodName,
				signature: params,
				async: funcNode.async || false,
				static: member.static || false,
				kind: member.kind
			};
		}
	}

	return methods;
}

function node_loader_trampoline_discover_klass_static_attributes(classNode, klass) {
	const attributes = {};

	if (!classNode.body || !classNode.body.body) {
		return attributes;
	}

	for (const member of classNode.body.body) {
		// Handle PropertyDefinition (ES2022 class fields)
		if (member.type === 'PropertyDefinition' && member.static) {
			// Skip if it's a method (arrow function assigned to property)
			if (member.value && member.value.type === 'ArrowFunctionExpression') {
				continue;
			}

			const attrName = member.key.type === 'Identifier'
				? member.key.name
				: (member.key.value || String(member.key.name));

			attributes[attrName] = {
				name: attrName,
				static: true
			};
		}
	}

	// Also check runtime properties on the class itself
	const ownProps = Object.getOwnPropertyNames(klass);
	for (const prop of ownProps) {
		if (prop === 'prototype' || prop === 'length' || prop === 'name') {
			continue;
		}
		const descriptor = Object.getOwnPropertyDescriptor(klass, prop);
		if (descriptor && typeof descriptor.value !== 'function') {
			if (!attributes[prop]) {
				attributes[prop] = {
					name: prop,
					static: true
				};
			}
		}
	}

	return attributes;
}

function node_loader_trampoline_discover_klass_instance_attributes(classNode) {
	const attributes = {};

	if (!classNode.body || !classNode.body.body) {
		return attributes;
	}

	// Find constructor and look for this.xxx = ... assignments
	const constructor = classNode.body.body.find(
		m => m.type === 'MethodDefinition' && m.kind === 'constructor'
	);

	if (constructor && constructor.value && constructor.value.body && constructor.value.body.body) {
		for (const stmt of constructor.value.body.body) {
			if (stmt.type === 'ExpressionStatement' &&
				stmt.expression.type === 'AssignmentExpression' &&
				stmt.expression.left.type === 'MemberExpression' &&
				stmt.expression.left.object.type === 'ThisExpression' &&
				stmt.expression.left.property.type === 'Identifier') {

				const attrName = stmt.expression.left.property.name;
				attributes[attrName] = {
					name: attrName,
					static: false
				};
			}
		}
	}

	// Also check for class field definitions (non-static PropertyDefinition)
	for (const member of classNode.body.body) {
		if (member.type === 'PropertyDefinition' && !member.static) {
			if (member.value && member.value.type === 'ArrowFunctionExpression') {
				continue;
			}

			const attrName = member.key.type === 'Identifier'
				? member.key.name
				: (member.key.value || String(member.key.name));

			attributes[attrName] = {
				name: attrName,
				static: false
			};
		}
	}

	return attributes;
}

function node_loader_trampoline_discover_klass(klass) {
	try {
		const str = klass.toString();
		const ast = espree.parse(`(${str})`, { ecmaVersion: 14 });
		const node = ast.body[0].type === 'ExpressionStatement'
			? ast.body[0].expression
			: ast.body[0];

		if (node.type !== 'ClassExpression' && node.type !== 'ClassDeclaration') {
			return undefined;
		}

		// Extract class name
		const className = node.id ? node.id.name : klass.name;

		// Discover constructors
		const constructors = node_loader_trampoline_discover_klass_constructors(node);

		// Discover own methods (with static detection via espree)
		const methods = node_loader_trampoline_discover_klass_methods(node, klass);

		// Discover static attributes
		const staticAttributes = node_loader_trampoline_discover_klass_static_attributes(node, klass);

		// Discover instance attributes
		const instanceAttributes = node_loader_trampoline_discover_klass_instance_attributes(node);

		return {
			ptr: klass,
			type: 'class',
			name: className,
			constructors: constructors,
			methods: methods,
			static_attributes: staticAttributes,
			instance_attributes: instanceAttributes
		};
	} catch (ex) {
		console.log(`Exception while parsing class '${klass?.name || klass}'`, ex);
		return undefined;
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
				const exported = exports[key];

				// Check if it's a class first (before function check since classes are also functions)
				if (node_loader_trampoline_is_class(exported)) {
					const classDescriptor = node_loader_trampoline_discover_klass(exported);
					if (classDescriptor !== undefined) {
						discover[key] = classDescriptor;
					}
				}
				// Check if it's a regular function
				else if (node_loader_trampoline_is_callable(exported)) {
					const funcDescriptor = node_loader_trampoline_discover_function(exported);
					if (funcDescriptor !== undefined) {
						discover[key] = funcDescriptor;
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
	// Imporant: never trigger an async resource in this function
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

		try {
			return Promise.resolve(func(...args)).then(
				x => trampoline.resolve(trampoline_ptr, x),
				x => trampoline.reject(trampoline_ptr, x),
			);
		} catch (err) {
			try {
				return trampoline.reject(trampoline_ptr, err);
			} catch (fatal) {
				return Promise.reject(fatal);
			}
		}
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

		try {
			return Promise.resolve(future).then(
				x => trampoline.resolve(trampoline_ptr, x),
				x => trampoline.reject(trampoline_ptr, x),
			);
		} catch (err) {
			try {
				return trampoline.reject(trampoline_ptr, err);
			} catch (fatal) {
				return Promise.reject(fatal);
			}
		}
	};
}

const startup = (impl, ptr, trampoline_exports) => {
	try {
		if (typeof impl === 'undefined' || typeof ptr === 'undefined') {
			throw new Error('Bootstrap startup arguments impl or ptr are not defined.');
		}

		// Get trampoline from list of linked bindings
		const trampoline = (() => {
			if (trampoline_exports) {
				return trampoline_exports;
			}

			return process._linkedBinding('node_loader_trampoline_module');
		})();

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
		});

		// This function must destroy all the loaders but
		// delaying the NodeJS Loader library unloading
		if (trampoline_exports) {
			process.on('exit', () => trampoline.destroy(node_loader_ptr));
		}
	} catch (ex) {
		console.log('Exception in bootstrap.js trampoline initialization:', ex);
	}
};

module.exports = ((impl, ptr) => {
	if (impl === undefined || ptr === undefined) {
		return startup;
	} else {
		return startup(impl, ptr);
	}
})(process.argv[2], process.argv[3]);
