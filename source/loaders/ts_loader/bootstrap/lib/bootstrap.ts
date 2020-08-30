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
const trampoline = require('./trampoline.node');

const Module = require('module');
const path = require('path');
const util = require('util');
const fs = require('fs');

const ts = require('./node_modules/typescript');

/**
 * Note: This variables should be stored into a object
 * returned in the initialization, and used in each function
 * when called as a first parameter, this object will live
 * during the lifetime of this loader and will represent the
 * global state of the boostrap script. In order to simplify
 * the design, by now they are used as global variables.
*/
let registry = null;
let servicesHost = null;
let services = null;

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
		// TODO: Use LOADER_SCRIPT_PATH ?
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

	clearFile(name) {
		const file = this.files[name];
		if (file) {
			// Dispose the snapshot if method is available
			if (file.snapshot.dispose) {
				file.snapshot.dispose();
				file.snapshot = null;
			}

			// Remove it from the storage
			delete this.files[name];
		}
	}

	dispose() {
		for (const file in this.files) {
			this.clearFile(file);
		}
	}
}

function ts_loader_trampoline_initialize() {
	// Initialize Language Service API
	registry = ts.createDocumentRegistry();
	servicesHost = new TypeScriptLanguageServiceHost();
	services = ts.createLanguageService(servicesHost, registry);
	const lib = path.dirname(servicesHost.getDefaultLibFileName(servicesHost.getCompilationSettings()));

	// Load TypeScript Runtime
	fs.readdirSync(lib).map(file => {
		const filename = path.join(lib, file);
		const stat = fs.lstatSync(filename);
		if (!stat.isDirectory() && file.startsWith('lib') && file.endsWith('.d.ts')) {
			servicesHost.addFile(filename);
		}
	});
}

function ts_loader_trampoline_is_callable(value) {
	return typeof value === 'function';
}

function ts_loader_trampoline_is_valid_symbol(node) {
	// TODO: Enable more function types
	return ts.isFunctionDeclaration(node.valueDeclaration);
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
			const emitName = emit.outputFiles[0].name;
			const name = `${emitName.substr(0, emitName.lastIndexOf('.'))}.ts`;
			handle[name] = ts_loader_trampoline_load_inline(p, emit.outputFiles[0].text, {});
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

	// TODO: Implement this with service host instead of transpile, it wont work with discovery
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

			// Clear file from NodeJS require cache
			if (require.cache[absolute]) {
				delete require.cache[absolute];
			}

			// Clear file from TypeScript service host
			servicesHost.clearFile(p);
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

function ts_loader_trampoline_discover_signature(checker, node) {
	const declaration = node.valueDeclaration;
	const signature = checker.getSignatureFromDeclaration(declaration);
	const params = declaration.parameters;
	const args = [];
	const types = [];

	// Get the function return type
	const ret = signature.getReturnType().intrinsicName || 'any';

	// Retrieve the arguments names and types
	for (let i = 0; i < params.length; ++i) {
		const param = params[i];
		const type = checker.getTypeAtLocation(param);
		args.push(param.name.escapedText || 'undefined');
		types.push(type.intrinsicName || 'any');
	}

	// Generate names for unnamed arguments
	for (let i = 0; i < args.length; ++i) {
		if (args[i] === 'undefined') {
			args[i] = ts_loader_trampoline_discover_arguments_generate(args, i);
		}
	}

	return {
		args,
		types,
		ret,
	};
}

function ts_loader_trampoline_discover(handle) {
	const discover = {};

	function getExportList(node, checker) {
		const symbol = checker.getSymbolAtLocation(node);
		return symbol ? checker.getExportsOfModule(symbol) : [];
	}

	try {
		const program = services.getProgram();
		const checker = program.getTypeChecker();
		const names = Object.getOwnPropertyNames(handle);
		const sources = program.getSourceFiles();
		const sourcesMap = sources.reduce((srcs, s) => Object.assign(srcs, { [s.fileName]: s }), {});
		const tsHandle = names.reduce((srcs, name) => Object.assign(srcs, { [name]: sourcesMap[name] }), {});

		for (let i = 0; i < names.length; ++i) {
			const name = names[i];
			const exports = handle[name];
			const tsFile = tsHandle[name]
			const tsExports = getExportList(tsFile, checker).reduce((ex, e) => Object.assign(ex, { [e.name]: e }), {});
			const keys = Object.getOwnPropertyNames(tsExports);

			for (let j = 0; j < keys.length; ++j) {
				const key = keys[j];
				const func = exports[key];

				if (ts_loader_trampoline_is_callable(func)) {
					const node = tsExports[key];
					const type = checker.getTypeAtLocation(node);

					if (ts_loader_trampoline_is_valid_symbol(node)) {
						const signature = ts_loader_trampoline_discover_signature(checker, node);
						const flags = ts.getCombinedModifierFlags(node.valueDeclaration);
						const isAsync = Boolean(flags & ts.ModifierFlags.Async);

						discover[key] = {
							ptr: func,
							signature: signature.args,
							types: signature.types,
							ret: signature.ret,
							async: isAsync,
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

		// Clear TypeScript Service API
		servicesHost.dispose();
		services.dispose();
		registry = null;
		servicesHost = null;
		services = null;
	} catch (ex) {
		console.log('Exception in ts_loader_trampoline_destroy', ex);
	}
}

module.exports = ((impl, ptr) => {
	return trampoline.register(impl, ptr, {
		'initialize': ts_loader_trampoline_initialize,
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
