#!/usr/bin/env node

const trampoline = require('./trampoline.node');

var Module = require('module');
var path = require('path');

function node_loader_trampoline_execution_path() {
	// TODO
}

function node_loader_trampoline_load_from_file(paths) {
	const handle = {};

	for (const path of paths) {
		const module = require(path);

		handle[path] = module;
	}

	return handle;
}

function node_loader_trampoline_load_from_memory(name, buffer, opts) {
	opts.prepend_paths = opts.prepend_paths || [];
	opts.append_paths = opts.append_paths || [];

	if (typeof buffer !== 'string') {
		throw new Error('Load from memory buffer must be a string, not ' + typeof code);
	}

	const paths = Module._nodeModulePaths(path.dirname(name));

	const parent = module.parent;

	const m = new Module(name, parent);

	m.filename = name;
	m.paths = [].concat(opts.prepend_paths).concat(paths).concat(opts.append_paths);
	m._compile(code, name);

	const exports = m.exports;

	if (parent && parent.children) {
		parent.children.splice(parent.children.indexOf(m), 1);
	}

	const handle = {};

	handle[name] = exports;

	return handle;
}

function node_loader_trampoline_load_from_package() {
	// TODO
}

function node_loader_trampoline_clear(handle) {
	for (const path of handle) {
		if (require.cache[path]) {
			delete require.cache[path];
		}
	}
}

function node_loader_trampoline_discover(handle) {
	// TODO
}

function node_loader_trampoline_test() {
	console.log('NodeJS Loader Bootstrap Test');
}

module.exports = ((ptr) => {
	return trampoline.register(ptr, {
		'execution_path': node_loader_trampoline_execution_path,
		'load_from_file': node_loader_trampoline_load_from_file,
		'load_from_memory': node_loader_trampoline_load_from_memory,
		'load_from_package': node_loader_trampoline_load_from_package,
		'clear': node_loader_trampoline_clear,
		'discover': node_loader_trampoline_discover,
		'test': node_loader_trampoline_test,
	});
})(process.argv[2]);
