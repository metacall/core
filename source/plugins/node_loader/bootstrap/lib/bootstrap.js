#!/usr/bin/env node

const trampoline = require('./trampoline.node');

module.exports = ((ptr) => {
	return trampoline.register(ptr);
})(process.argv[2]);
