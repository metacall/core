#!/usr/bin/env sh
':' //; exec "$(command -v nodejs || command -v node)" "$0" "$@"

function say_divide(a :: Number, b :: Number) :: Number {
	return (a / b);
}

function some_text(a :: String, b :: String) :: String {
	return (a + b);
}
