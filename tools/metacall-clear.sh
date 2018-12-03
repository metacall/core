#!/bin/bash

#
# MetaCall Configuration Environment Bash Script by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Remove all packages and unused data from MetaCall building and testing.
#

ROOT_DIR=$(pwd)

RUN_AS_ROOT=0
SUDO_CMD=sudo
CLEAR_RAPIDJSON=0
CLEAR_FUNCHOOK=0
CLEAR_PYTHON=0
CLEAR_RUBY=0
CLEAR_NETCORE=0
CLEAR_V8=0
CLEAR_NODEJS=0
CLEAR_APT=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Base packages
sub_apt(){
	echo "clean apt of C build"
	$SUDO_CMD apt-get -y remove --purge build-essential git cmake wget apt-utils apt-transport-https gnupg dirmngr ca-certificates
	$SUDO_CMD apt-get -y autoclean
	$SUDO_CMD apt-get -y autoremove
}

# RapidJSON
sub_rapidjson(){
	echo "clean rapidJSON"
	$SUDO_CMD rm -rf /usr/local/lib/cmake
	$SUDO_CMD rm -rf /usr/local/include/rapidjson
}

# FuncHook
sub_funchook(){
	echo "clean funchook"
}

# Python
sub_python(){
	echo "clean python"
	/usr/bin/yes | $SUDO_CMD pip3 uninstall django
	/usr/bin/yes | $SUDO_CMD pip3 uninstall requests
	/usr/bin/yes | $SUDO_CMD pip3 uninstall rsa
}

# Ruby
sub_ruby(){
	echo "clean ruby"
	# TODO: Review conflict with NodeJS (currently rails test is disabled)
	#$SUDO_CMD gem uninstall rails
	#$SUDO_CMD apt-get -y remove --purge nodejs
}

# NetCore
sub_netcore(){
	echo "clean netcore"
	$SUDO_CMD apt-get -y remove --purge libssl1.0-dev libkrb5-dev clang
}

# V8
sub_v8(){
	echo "clean v8"
}

# NodeJS
sub_nodejs(){
	echo "clean nodejs"
}

# MetaCall
sub_metacall(){
	echo "clean metacall"
	$SUDO_CMD rm -rf /usr/local/share/metacall/data
	$SUDO_CMD rm /usr/local/share/metacall/VERSION
	$SUDO_CMD rm /usr/local/share/metacall/README
	$SUDO_CMD rm /usr/local/share/metacall/metacall-config.cmake
}

# Clear
sub_clear(){
	if [ $RUN_AS_ROOT = 1 ]; then
		SUDO_CMD=""
	fi
	if [ $CLEAR_RAPIDJSON = 1 ]; then
		sub_rapidjson
	fi
	if [ $CLEAR_FUNCHOOK = 1 ]; then
		sub_funchook
	fi
	if [ $CLEAR_PYTHON = 1 ]; then
		sub_python
	fi
	if [ $CLEAR_RUBY = 1 ]; then
		sub_ruby
	fi
	if [ $CLEAR_NETCORE = 1 ]; then
		sub_netcore
	fi
	if [ $CLEAR_V8 = 1 ]; then
		sub_v8
	fi
	if [ $CLEAR_NODEJS = 1 ]; then
		sub_nodejs
	fi
	if [ $CLEAR_APT = 1 ]; then
		sub_apt
	fi

	sub_metacall

	# Delete MetaCall path
	rm -rf $METACALL_PATH

	echo "clean finished in workspace $ROOT_DIR"
}

# Configuration
sub_options(){
	for var in "$@"
	do
		if [ "$var" = 'root' ]; then
			echo "running as root"
			RUN_AS_ROOT=1
		fi
		if [ "$var" = 'rapidjson' ]; then
			echo "rapidjson selected"
			CLEAR_RAPIDJSON=1
		fi
		if [ "$var" = 'funchook' ]; then
			echo "funchook selected"
			CLEAR_FUNCHOOK=1
		fi
		if [ "$var" = 'python' ]; then
			echo "python selected"
			CLEAR_PYTHON=1
		fi
		if [ "$var" = 'ruby' ]; then
			echo "ruby selected"
			CLEAR_RUBY=1
		fi
		if [ "$var" = 'netcore' ]; then
			echo "netcore selected"
			CLEAR_NETCORE=1
		fi
		if [ "$var" = 'v8' ]; then
			echo "v8 selected"
			CLEAR_V8=1
		fi
		if [ "$var" = 'nodejs' ]; then
			echo "nodejs selected"
			CLEAR_NODEJS=1
		fi
		if [ "$var" = 'base' ]; then
			echo "apt selected"
			CLEAR_APT=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: $PROGNAME list of component"
	echo "Components:"
	echo "	root"
	echo "	base"
	echo "	rapidjson"
	echo "	funchook"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	v8"
	echo "	nodejs"
	echo ""
}

case "$#" in
	0)
		sub_help
		;;
	*)
		sub_options $@
		sub_clear
		;;
esac
