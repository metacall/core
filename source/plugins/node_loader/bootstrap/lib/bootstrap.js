#!/usr/bin/env node

const trampoline = require('./trampoline.node');

function node_loader_trampoline_test() {
	console.log('NodeJS Loader Bootstrap Test');
}

module.exports = ((ptr) => {
	return trampoline.register(ptr, {
		'test': node_loader_trampoline_test,
	});
})(process.argv[2]);
