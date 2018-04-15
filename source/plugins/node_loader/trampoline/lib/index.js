#!/usr/bin/env node

const binding = require('binding')('trampoline');

module.exports = function register(ptr) {
	return binding.register(ptr);
};
