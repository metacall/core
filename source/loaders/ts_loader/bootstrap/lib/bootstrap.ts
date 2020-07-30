#!/usr/bin/env node
'use strict';

/**
 * Note: This file is called bootstrap.ts but it must
 * be fully compatible with NodeJS. We are using this
 * name only to avoid name collisions with bootstrap.js
 * but this script is launched by NodeJS and it should
 * not be transpiled or implemented in TypeScript
*/

// eslint-disable-next-line global-require
// TODO: Trampoline
/*
const trampoline = require('./trampoline.node');
*/

const Module = require('module');
const path = require('path');
const util = require('util');
const fs = require('fs');

const ts = require('./node_modules/typescript');

class TypeScriptLanguageServiceHost {
	constructor() {
		this.files = {};
	}

	fileExists() {
		return ts.sys.fileExists(...arguments);
	}

	readFile() {
		return ts.sys.readFile(...arguments);
	}

	readDirectory() {
		return ts.sys.readDirectory(...arguments);
	}

	log(message) {
		// TODO: Improve this
		console.log(message);
	}

	trace(message) {
		// TODO: Improve this
		console.log(message);
	}

	error(message) {
		// TODO: Improve this
		console.log(message);
	}

	getCompilationSettings() {
		return ts.getDefaultCompilerOptions();
	}

	getCurrentDirectory() {
		// TODO: Return script path
		return process.cwd();
	}

	getDefaultLibFileName(options) {
		return ts.getDefaultLibFilePath(options);
	}

	getScriptVersion(name) {
		return this.files[name] && this.files[name].version.toString();
	}

	getScriptSnapshot(name) {
		return this.files[name] && this.files[name].snapshot;
	}

	getScriptFileNames() {
		const names = [];
		for (const name in this.files) {
			if (this.files.hasOwnProperty(name)) {
				names.push(name);
			}
		}
		return names;
	}

	addFile(name) {
		const resolve = path.resolve(__dirname, name);
		const body = fs.readFileSync(resolve).toString();
		const snapshot = ts.ScriptSnapshot.fromString(body);

		snapshot.getChangeRange = _ => undefined;

		const file = this.files[name];

		if (file) {
			file.version++;
			file.snapshot = snapshot;
		} else {
			this.files[name] = {
				version: 1,
				snapshot: snapshot,
			};
		}
	}

	getFile(name) {
		return this.files[name];
	}
}

// Initialize Language Service API
const registry = ts.createDocumentRegistry();
const servicesHost = new TypeScriptLanguageServiceHost();
const services = ts.createLanguageService(servicesHost, registry);
const lib = path.dirname(servicesHost.getDefaultLibFileName(servicesHost.getCompilationSettings()));

// Load TypeScript Runtime
fs.readdirSync(lib).map(file => {
	const filename = path.join(lib, file);
	const stat = fs.lstatSync(filename);
	if (!stat.isDirectory() && file.startsWith('lib') && file.endsWith('.d.ts')) {
		servicesHost.addFile(filename);
	}
});

function ts_loader_trampoline_is_callable(value) {
	return typeof value === 'function';
}

function ts_loader_trampoline_is_valid_symbol(node) {
	// TODO: Enable more function types
	return node.type === 'FunctionExpression' || node.type === 'ArrowFunctionExpression';
}

function ts_loader_trampoline_module(m) {
	if (ts_loader_trampoline_is_callable(m)) {
		const wrapper = {};

		wrapper[m.name] = m;

		return wrapper;
	}

	return m;
}

function ts_loader_trampoline_load_inline(name, buffer, opts) {
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

	return ts_loader_trampoline_module(m.exports);
}

// eslint-disable-next-line no-empty-function
function ts_loader_trampoline_execution_path() {
	// TODO
}

function ts_loader_trampoline_load_from_file(paths) {
	if (!Array.isArray(paths)) {
		throw new Error('Load from file paths must be an array, not ' + typeof paths);
	}

	try {
		const handle = {};

		for (let i = 0; i < paths.length; ++i) {
			const p = paths[i];
			servicesHost.addFile(p);
			const emit = services.getEmitOutput(p);
			handle[p] = ts_loader_trampoline_load_inline(p, emit.outputFiles[0].text, {});
		}

		return handle;
	} catch (ex) {
		console.log('Exception in ts_loader_trampoline_load_from_file', ex);
	}

	return null;
}

function ts_loader_trampoline_load_from_memory(name, buffer, opts) {
	if (typeof name !== 'string') {
		throw new Error('Load from memory name must be a string, not ' + typeof name);
	}

	if (typeof buffer !== 'string') {
		throw new Error('Load from memory buffer must be a string, not ' + typeof buffer);
	}

	if (typeof opts !== 'object') {
		throw new Error('Load from memory opts must be an object, not ' + typeof opts);
	}

	const handle = {};

	handle[name] = ts_loader_trampoline_load_inline(name, ts.transpile(buffer), opts);

	return handle;
}

// eslint-disable-next-line no-empty-function
function ts_loader_trampoline_load_from_package() {
	// TODO
}

function ts_loader_trampoline_clear(handle) {
	try {
		const names = Object.getOwnPropertyNames(handle);

		for (let i = 0; i < names.length; ++i) {
			const p = names[i];
			const absolute = path.resolve(__dirname, p);

			if (require.cache[absolute]) {
				delete require.cache[absolute];
			}

			// TODO: TypeScript: Clear from service host
		}
	} catch (ex) {
		console.log('Exception in ts_loader_trampoline_clear', ex);
	}
}

function ts_loader_trampoline_discover_arguments_generate(args, index) {
	let name = `_arg${index}`;

	while (args.includes(name)) {
		name = `_${name}`;
	}

	return name;
}

function ts_loader_trampoline_discover_arguments(node) {
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
			args[i] = ts_loader_trampoline_discover_arguments_generate(args, i);
		}
	}

	return args;
}

function ts_loader_trampoline_discover(handle) {
	const discover = {};

	try {
		const names = Object.getOwnPropertyNames(handle);

		for (let i = 0; i < names.length; ++i) {
			const exports = handle[names[i]];
			const keys = Object.getOwnPropertyNames(exports);

			// Remove private key generated from transpilation process
			keys.splice(keys.indexOf('__esModule'), 1);

			for (let j = 0; j < keys.length; ++j) {
				const key = keys[j];
				const func = exports[key];

				if (ts_loader_trampoline_is_callable(func)) {
					// TODO: Store AST in the handle
					const ast = cherow.parse(`(${func.toString()})`, {
						module: false,
						skipShebang: true,
					}).body[0];

					const node = ast.expression;

					if (ts_loader_trampoline_is_valid_symbol(node)) {
						const args = ts_loader_trampoline_discover_arguments(node);

						discover[key] = {
							ptr: func,
							signature: args,
							async: node.async,
						};
					}
				}
			}
		}
	} catch (ex) {
		console.log('Exception in ts_loader_trampoline_discover', ex);
	}

	return discover;
}

function ts_loader_trampoline_test(obj) {
	console.log('NodeJS Loader Bootstrap Test');

	if (obj !== undefined) {
		console.log(util.inspect(obj, false, null, true));
	}
}

function ts_loader_trampoline_await(func, args, trampoline_ptr) {
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

function ts_loader_trampoline_destroy() {
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
		console.log('Exception in ts_loader_trampoline_destroy', ex);
	}
}

// TODO: Trampoline
/*
module.exports = ((impl, ptr) => {
	return trampoline.register(impl, ptr, {
		'execution_path': ts_loader_trampoline_execution_path,
		'load_from_file': ts_loader_trampoline_load_from_file,
		'load_from_memory': ts_loader_trampoline_load_from_memory,
		'load_from_package': ts_loader_trampoline_load_from_package,
		'clear': ts_loader_trampoline_clear,
		'discover': ts_loader_trampoline_discover,
		'test': ts_loader_trampoline_test,
		'await': ts_loader_trampoline_await,
		'destroy': ts_loader_trampoline_destroy,
	});
})(process.argv[2], process.argv[3]);
*/

// TEST
ts_loader_trampoline_load_from_memory('hello', `
export function lol(left: number, rigth: number): number {
	return left + rigth;
}
`, {});
const handle = ts_loader_trampoline_load_from_file(['./test.ts']);
const discover = ts_loader_trampoline_discover(handle);

console.log(discover);
