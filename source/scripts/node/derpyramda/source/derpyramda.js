#!/usr/bin/env node

function add(x, y) {
	return Number(x) + Number(y);
}

function subtract(x, y) {
	return Number(x) - Number(y);
}

function flip(fn) {
	return function (x, y) {
		return fn(y, x);
	};
}

module.exports = {
	add,
	subtract,
	flip,
};
