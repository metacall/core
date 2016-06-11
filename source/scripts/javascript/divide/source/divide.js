#!/bin/sh
':' //; exec "$(command -v nodejs || command -v node)" "$0" "$@"

function say_divide(a, b) {
	return (a / b);
}
