#!/usr/bin/env node

const binding = require('bindings')('trampoline');

function node_loader_trampoline_test() {

}

module.exports = function register(ptr) {
	return binding.register(ptr, {
		'test': node_loader_trampoline_test,
	});
};
