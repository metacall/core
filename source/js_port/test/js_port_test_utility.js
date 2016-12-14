//#!/usr/bin/d8

function test(t) {
	quit(t());
}

function test_case(t) {
	t();
}

function assert(left, right) {
	if (left !== right)
	{
		print('Assert error: ' + left + ' !== ' + right);

		quit(1);
	}
}
