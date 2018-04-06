#!/usr/bin/env node

/*
var addon = require('bindings')('node_loader_addon');

console.log(addon.hello()); // 'world'
*/

var counter = 0;

console.log("HOOOOOOOOOLAAA JOSEEEEEEEEEEEE");

const intervalObj = setInterval(() => {
	counter++;

	if (counter == 10) {
		clearInterval(intervalObj);
	}
}, 1000);