#!/usr/bin/env node

function hello_boy(a, b) {
	console.log('Hey boy!!');
	hello_boy_await(4, 5).then(result => process.stdout.write(`Result: ${result}.`));
	console.log('Hoy boy!!');
	return (a + b);
}

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

async function hello_boy_await(a, b) {
	await sleep(50);
	console.log('Hey await boy!!');
	return (a + b);
}

async function hello_boy_nested_await(a, b) {
	try {
		await sleep(50);
		return (a + b);
	} catch (e) {
		return 'Fail';
	}
}

async function throw_await() {
	throw 'Hello World';
}

async function return_await() {
	return 'Hello World';
}

function call_test(a, b) {
	return (a * b);
}

const lambda = () => {
	return 15;
};

function object_pattern({a}, {b}, {c}) {
	return 16;
}

function object_pattern_evil_left({a}, {b}, {c}, _arg0, _arg1, _arg2) {
	return 17;
}

function object_pattern_evil_right(_arg0, _arg1, _arg2, {a}, {b}, {c}) {
	return 18;
}

function object_pattern_evil_repeat({a}, _arg0, __arg0, ___arg0) {
	return 19;
}

function default_values(a = 1234) {
	return 20;
}

const all_mixed = ({a}, _arg0, b = 'asd') => {
	return 21;
};

function this_function_should_not_be_exported() {
	return 5;
}

async function verify_jwt(url, token) {
	console.log(url, token);
	await sleep(1);
	return true;
}

const verify_func = (url, f) => async (token, serial, range) => {
	if (await verify_jwt(url, token)) {
		return await f(serial, range);
	} else {
		return { error: 'Failed to verify token' };
	}
}

async function get_random_data(serial, range) {
	console.log(serial, range);
	await sleep(1);
	return 12.0;
}

module.exports = {
	hello_boy,
	hello_boy_await,
	hello_boy_nested_await,
	throw_await,
	return_await,
	call_test,
	lambda,
	inline_lambda: (a, b) => {},
	object_pattern,
	object_pattern_evil_left,
	object_pattern_evil_right,
	object_pattern_evil_repeat,
	default_values,
	all_mixed,
	/* this_function_should_not_be_exported, */
	get_random_data: verify_func('https://some.random.serivce.to.verify.jwt.com', get_random_data),
};
