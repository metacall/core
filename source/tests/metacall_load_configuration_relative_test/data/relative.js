#!/usr/bin/env node

//const wtf = require('./wtfnode');

//let curr_value = 0;

function hello_boy(a, b) {
	console.log('Hey boy!!');
	return (a + b);
}

function call_test(a, b) {
	//console.log(`${curr_value} ======================================================`);
	//console.log(process._getActiveHandles().map(x => x.constructor.name));
	//console.log(process._getActiveHandles());
	//wtf.dump();
	//curr_value++;
	return (a * b);
}

module.exports = {
	hello_boy,
	call_test,
};
