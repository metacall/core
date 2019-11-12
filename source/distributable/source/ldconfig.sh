#!/usr/bin/env sh

if [ ! "`command -v ldconfig`" = "" ]; then
	ldconfig $1
else
	echo "Warning:"
	echo "    MetaCall Distributable could not be registered properly."
	echo "    Some tests depending on this library (like NodeJS Port) could fail."
	echo "    You may want to run this command (build or install) with sudo again to solve this issue."
fi
