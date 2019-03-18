#!/usr/bin/env node
'use strict';

const { dirname, resolve } = require('path');
const Module = require('module');

const { parse } = require('espree');

// eslint-disable-next-line global-require
const trampoline = require('./trampoline.node');

const clear = handle =>
	Object.keys(handle).forEach(p =>
		delete require.cache[p]);

const destroy = () =>
	// eslint-disable-next-line no-underscore-dangle
	process._getActiveHandles().forEach(h => {
		// eslint-disable-next-line no-param-reassign, no-empty-function
		h.write = () => {};
		// eslint-disable-next-line max-len
		// eslint-disable-next-line no-param-reassign, no-underscore-dangle, no-empty-function
		h._destroy = () => {};
		return h.end && h.end();
	});

const prefixArg = (args, index, n = 0) => {
	const name = '_'.repeat(n) + 'arg' + index;
	return args.includes(name)
		? prefixArg(args, index, n + 1)
		: name;
};

const setUnnamedArgs = args =>
	args.map((a, i) =>
		typeof a === 'string'
			? a
			: prefixArg(args, i));

const discoverArguments = fun => {
	if (typeof fun !== 'function') {
		throw new TypeError(fun.name + ' is not a function');
	}
	const { body: [ f ] } =
		parse(`(\n${fun.toString()}\n)`, { ecmaVersion: 2019 });
	if (
		f.type !== 'FunctionDeclaration' &&
		f.type !== 'ExpressionStatement' &&
		f.expression.type !== 'ArrowFunctionExpression'
	) {
		throw new TypeError(fun.name + ' is not a function');
	}
	return setUnnamedArgs((f.params || f.expression.params).map(p =>
		/* eslint-disable no-nested-ternary, operator-linebreak, indent */
		p.type === 'Identifier' ? p.name :
		p.type === 'AssignmentPattern' &&
		p.left.type === 'Identifier' ? p.left.name :
		false));
};

const discover = handle =>
	Object.values(handle).reduce((discovered, exports) => ({
		...discovered,
		...Object.entries(exports).reduce((acc, [ key, value ]) => ({
			...acc,
			[key]: {
				ptr: value,
				signature: discoverArguments(value)
			}
		}), {})
	}), {});

// eslint-disable-next-line no-empty-function
const execution_path = () => {}; // TODO
// eslint-disable-next-line no-empty-function
const load_from_package = () => {}; // TODO

const test = () =>
	console.log('NodeJS Loader Bootstrap Test');

const ensureArray = x =>
	Array.isArray(x) ? x : [ x ];

const load_from_file = paths =>
	ensureArray(paths).reduce((acc, path) => {
		const abs = resolve(__dirname, path);
		return {
			...acc,
			// eslint-disable-next-line global-require
			[abs]: require(abs)
		};
	}, {});

const node_loader_trampoline_is_class = value => {
	// TODO
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
};

const node_loader_trampoline_is_generator = value =>
	value &&
	typeof value.next === 'function' &&
	typeof value.throw === 'function';

const node_loader_trampoline_is_generator_function = value =>
	typeof value === 'function' &&
	value.constructor &&
	value.constructor.name === 'GeneratorFunction';

const node_loader_trampoline_is_callable = (has_to_string_tag => {
	// TODO

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
	}

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

		return str === '[object Function]' ||
			str === '[object GeneratorFunction]';
	};

})(typeof Symbol === 'function' && typeof Symbol.toStringTag === 'symbol');

const node_loader_trampoline_is_arrow_function = value => {
	// TODO
	if (!node_loader_trampoline_is_callable(value)) {
		return false;
	}

	try {
		const re_function = /^\s*function/;
		const re_arrow_with_parents = /^\([^)]*\) *=>/;
		const re_arrow_without_parents = /^[^=]*=>/;

		const str = Function.prototype.toString.call(value);

		return str.length > 0 &&
			!re_function.test(str) &&
			(re_arrow_with_parents.test(str) ||
			re_arrow_without_parents.test(str));
	} catch (ex) {
		console.log(
			'Exception in node_loader_trampoline_is_arrow_function',
			ex);
	}

	return false;
};

const node_loader_trampoline_is_valid_symbol = value => {
	// TODO
	if (node_loader_trampoline_is_class(value)) {
		console.log(
			'Exception in node_loader_trampoline_is_valid',
			'classes are not suported');
		return false;
	}

	if (
		node_loader_trampoline_is_generator(value) ||
		node_loader_trampoline_is_generator_function(value)
	) {
		console.log(
			'Exception in node_loader_trampoline_is_valid',
			'anonymous generator (functions) are not suported');
		return false;
	}

	if (node_loader_trampoline_is_arrow_function(value)) {
		console.log(
			'Exception in node_loader_trampoline_is_valid',
			'anonymous arrow functions are not suported');
		return false;
	}

	/* TODO: Add extra type detection */

	return true;
};


const t_module = m => {
	// TODO
	if (!node_loader_trampoline_is_valid_symbol(m)) {
		return {};
	}

	if (node_loader_trampoline_is_callable(m)) {
		const wrapper = {};

		wrapper[m.name] = m;

		return wrapper;
	}

	return m;
};


const load_from_memory = (name, buffer, opts = {}) => {
	// TODO
	const {
		prepend_paths = [],
		append_paths = []
	} = opts;

	// eslint-disable-next-line no-underscore-dangle
	const paths = Module._nodeModulePaths(dirname(name));
	const { parent } = module;
	const m = new Module(name, parent);

	m.filename = name;
	m.paths = [
		...prepend_paths,
		...paths,
		...append_paths
	];

	// eslint-disable-next-line no-underscore-dangle
	m._compile(String(buffer), name);

	const { exports } = m;

	if (parent && parent.children) {
		parent.children.splice(parent.children.indexOf(m), 1);
	}

	return { [name]: t_module(exports) };

};

const [ impl, ptr ] = process.argv.slice(2);

module.exports = trampoline.register(impl, ptr, {
	clear,
	destroy,
	discover,
	execution_path,
	load_from_file,
	load_from_memory,
	load_from_package,
	test
});
