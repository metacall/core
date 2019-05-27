#!/usr/bin/env sh
':' //; exec "$(command -v nodejs || command -v node)" "$0" "$@"

function say_divide(a, b) {
	return (a / b);
}

function some_text(a, b) {
	return (a + b);
}

function mixed_js(a, b :: Number, c :: Number, d) :: Number {
	return (a + b + c + d);
}

function mixed_noreturn(a, b :: Number, c :: Number, d) {
	return (a + b + c + d);
}
