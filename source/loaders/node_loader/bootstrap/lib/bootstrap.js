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

const parseFunc = f => {
	if (typeof f !== 'function') {
		return null;
	}
	return parse(
		`(\n${f.toString()}\n)`,
		{ ecmaVersion: 2019 }).body[0];
};

const discoverArguments = fun => {
	const f = parseFunc(fun);
	if (
		!f ||
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

const is_callable = value =>
	typeof value === 'function';

const t_module = m =>
	is_callable(m)
			? { [m.name]: m }
			: m;

const load_from_memory = (name, buffer, opts = {}) => {

	const { parent } = module;
	const m = Object.assign(new Module(name, parent), {
		filename: name,
		paths: [
			...opts.prepend_paths || [],
			// eslint-disable-next-line no-underscore-dangle
			...Module._nodeModulePaths(dirname(name)),
			...opts.append_paths || []
		]
	});

	if (parent && parent.children) {
		parent.children.splice(parent.children.indexOf(m), 1);
	}

	// eslint-disable-next-line no-underscore-dangle
	m._compile(String(buffer), name);

	return { [name]: t_module(m.exports) };
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
