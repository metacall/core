#!/usr/bin/env node

function pluginC() {
	console.log('Hello World, from extensionC');
	return 8;
}

module.exports = {
	pluginC
};
