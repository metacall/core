#!/bin/sh
':' //; exec "$(command -v nodejs || command -v node)" "$0" "$@"

function third_value(a :: String, b :: String, c :: String) :: String {
	return c;
}
