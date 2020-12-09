#!/usr/bin/env node

const js_function_chain = function(x) {
	return function(n) {
		console.log('------------------ js chain', n);
		console.log('------------------ js chain pre x() call', x.toString().slice(0, 12), '...');
		const result = x(x)(n);
		console.log('------------------ js chain post x() call', x.toString().slice(0, 12), '...');
		return result;
	};
};
const js_function_factorial = function(x) {
	return function(n) {
		console.log('------------------ js factorial', n);
		if (n == 0) {
			console.log('------------------ js factorial case base');
			return 1;
		} else {
			console.log('------------------ js factorial pre x() call', x.toString().slice(0, 12), '...');
			const result = n * x(x)(n - 1);
			console.log('------------------ js factorial post x() call', x.toString().slice(0, 12), '...');
			return result;
		}
	};
}

module.exports = {
	js_function_chain,
	js_function_factorial,
};
