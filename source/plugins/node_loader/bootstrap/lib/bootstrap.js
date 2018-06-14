#!/usr/bin/env node

const trampoline = require('./trampoline.node');

const Module = require('module');
const path = require('path');

const node_loader_trampoline_is_class = ((value) => {
	const re = /^\s*class /;

	try {
		const stripped = Function.prototype.toString.call(value)
			.replace(/\/\/.*\n/g, '')
			.replace(/\/\*[.\s\S]*\*\//g, '')
			.replace(/\n/mg, ' ')
			.replace(/ {2}/g, ' ');

		return re.test(stripped);
	} catch (ex) {
		return false;
	}
});

const node_loader_trampoline_is_callable = ((has_to_string_tag) => {

	function node_loader_trampoline_try_call(value) {
		try {
			if (node_loader_trampoline_is_class(value)) {
				return false;
			}
			Function.prototype.toString.call(value);
			return true;
		} catch (e) {
			return false;
		}
	};

	return function node_loader_trampoline_is_callable(value) {
		if (!value) {
			return false;
		}

		if (typeof value !== 'function' && typeof value !== 'object') {
			return false;
		}

		if (has_to_string_tag) {
			return node_loader_trampoline_try_call(value);
		}

		if (node_loader_trampoline_is_class(value)) {
			return false;
		}

		const str = Object.prototype.toString.call(value);

		return str === '[object Function]' || str === '[object GeneratorFunction]';
	};

})(typeof Symbol === 'function' && typeof Symbol.toStringTag === 'symbol');

const node_loader_trampoline_is_generator = ((value) => {
	return value && typeof value.next === 'function' && typeof value.throw === 'function';
});

const node_loader_trampoline_is_generator_function = ((value) => {
	return typeof value === 'function' && value.constructor && value.constructor.name === 'GeneratorFunction';
});

const node_loader_trampoline_is_arrow_function = ((value) => {
	if (!node_loader_trampoline_is_callable(value)) {
		return false;
	}

	try {
		const re_function = /^\s*function/;
		const re_arrow_with_parents = /^\([^)]*\) *=>/;
		const re_arrow_without_parents = /^[^=]*=>/;

		const str = Function.prototype.toString.call(value);

		return str.length > 0 && !re_function.test(str) &&
			(re_arrow_with_parents.test(str) || re_arrow_without_parents.test(str));
	} catch(ex) {
		console.log('Exception in node_loader_trampoline_is_arrow_function', ex);
	}

	return false;
});

const node_loader_trampoline_is_valid_symbol = ((value) => {
	if (node_loader_trampoline_is_class(value)) {
		console.log('Exception in node_loader_trampoline_is_valid', 'classes are not suported');
		return false;
	}

	if (node_loader_trampoline_is_generator(value) || node_loader_trampoline_is_generator_function(value)) {
		console.log('Exception in node_loader_trampoline_is_valid', 'anonymous generator (functions) are not suported');
		return false;
	}

	if (node_loader_trampoline_is_arrow_function(value)) {
		console.log('Exception in node_loader_trampoline_is_valid', 'anonymous arrow functions are not suported');
		return false;
	}

	/* TODO: Add extra type detection */

	return true;
})

const node_loader_trampoline_module = ((m) => {

	if (!node_loader_trampoline_is_valid_symbol(m)) {
		return {};
	}

	if (node_loader_trampoline_is_callable(m)) {
		const wrapper = {};

		wrapper[m.name] = m;

		return wrapper;
	}

	return m;
});

function node_loader_trampoline_execution_path() {
	// TODO
}

function node_loader_trampoline_load_from_file(paths) {
	const handle = {};

	if (!Array.isArray(paths)) {
		throw new Error('Load from file paths must be an array, not ' + typeof code);
	}

	try {
		for (const p of paths) {
			const m = require(path.resolve(__dirname, p));

			handle[p] = node_loader_trampoline_module(m);
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_load_from_file', ex);
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

	handle[name] = node_loader_trampoline_module(exports);

	return handle;
}

function node_loader_trampoline_load_from_package() {
	// TODO
}

function node_loader_trampoline_clear(handle) {
	try {
		for (const p of Object.getOwnPropertyNames(handle)) {
			const absolute = path.resolve(__dirname, p);

			if (require.cache[absolute]) {
				delete require.cache[absolute];
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_clear', ex);
	}
}

function node_loader_trampoline_discover_arguments(fun) {
	try {
		const names = fun.toString()
			.match(/^[\s\(]*function[^(]*\(([^)]*)\)/)[1]
			.replace(/\/\/.*?[\r\n]|\/\*(?:.|[\r\n])*?\*\//g, '')
			.replace(/\s+/g, '').split(',');

		return names.length == 1 && !names[0] ? [] : names;
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_discover_arguments', ex);
	}
}

function node_loader_trampoline_discover(handle) {
	const discover = {};

	try {
		for (const p of Object.getOwnPropertyNames(handle)) {
			const exports = handle[p];

			for (const key of Object.getOwnPropertyNames(exports)) {
				const value = exports[key];

				if (node_loader_trampoline_is_valid_symbol(value) && node_loader_trampoline_is_callable(value)) {
					const args = node_loader_trampoline_discover_arguments(value);

					discover[key] = {
						ptr: value,
						signature: args,
					};
				}
			}
		}
	} catch (ex) {
		console.log('Exception in node_loader_trampoline_discover', ex);
	}

	console.log('Debug node_loader_trampoline_discover', discover);

	return discover;
}

function node_loader_trampoline_test() {
	console.log('NodeJS Loader Bootstrap Test');
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
	});
})(process.argv[2], process.argv[3]);
