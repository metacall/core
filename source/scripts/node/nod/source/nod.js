#!/usr/bin/env node

function hello_boy(a, b) {
	console.log('Hey boy!!');
	return (a + b);
}

function call_test(a, b) {
	return (a * b);
}

module.exports = {
	hello_boy,
	call_test,
};
