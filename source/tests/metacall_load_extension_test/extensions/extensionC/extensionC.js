#!/usr/bin/env node

function extensionC() {
	console.log('Hello World, from extensionC');
	return 8;
}

module.exports = {
	extensionC
};
