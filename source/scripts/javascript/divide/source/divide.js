#!/bin/sh
':' //; exec "$(command -v nodejs || command -v node)" "$0" "$@"

function say_divide(a :: Number, b :: Number) :: Number {
	return (a / b);
}
