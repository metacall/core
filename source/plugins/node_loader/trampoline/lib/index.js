#!/usr/bin/env node

const binding = require('bindings')('trampoline');

module.exports = function register(ptr) {
	return binding.register(ptr);
};
