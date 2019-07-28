#!/usr/bin/env bash

#
#	MetaCall Configuration Environment Bash Script by Parra Studios
#	Remove all packages and unused data from MetaCall building and testing.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

ROOT_DIR=$(pwd)

RUN_AS_ROOT=0
SUDO_CMD=sudo
CLEAR_APT=0
CLEAR_RAPIDJSON=0
CLEAR_FUNCHOOK=0
CLEAR_PYTHON=0
CLEAR_RUBY=0
CLEAR_NETCORE=0
CLEAR_V8=0
CLEAR_NODEJS=0
CLEAR_FILE=0
CLEAR_SWIG=0
CLEAR_PACK=0
CLEAR_COVERAGE=0
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
	/usr/bin/yes | $SUDO_CMD pip3 uninstall joblib
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

# File
sub_file(){
	echo "clean file"
}

# SWIG
sub_swig(){
	echo "clean swig"
	$SUDO_CMD apt-get -y remove --purge libpcre3-dev swig
}

# MetaCall
sub_metacall(){
	echo "clean metacall"
	$SUDO_CMD rm -rf /usr/local/share/metacall/data
	$SUDO_CMD rm /usr/local/share/metacall/VERSION
	$SUDO_CMD rm /usr/local/share/metacall/README.md
	$SUDO_CMD rm /usr/local/share/metacall/metacall-config.cmake
}

# Pack
sub_pack(){
	echo "clean pack"
	$SUDO_CMD apt-get -y remove --purge rpm

	# Uninstall Python Port Dependencies (TODO: This must be removed when enabled the pip3 install metacall)
	if [ $CLEAR_PYTHON = 1 ]; then
		/usr/bin/yes | $SUDO_CMD pip3 uninstall setuptools
	fi
}

# Coverage
sub_coverage(){
	echo "clean pack"
	$SUDO_CMD apt-get -y remove --purge lcov
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
	if [ $CLEAR_FILE = 1 ]; then
		sub_file
	fi
	if [ $CLEAR_SWIG = 1 ]; then
		sub_swig
	fi
	if [ $CLEAR_PACK = 1 ]; then
		sub_pack
	fi
	if [ $CLEAR_COVERAGE = 1 ]; then
		sub_coverage
	fi

	# Clear aptitude (must be at the end)
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
		if [ "$var" = 'base' ]; then
			echo "apt selected"
			CLEAR_APT=1
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
		if [ "$var" = 'file' ]; then
			echo "file selected"
			CLEAR_FILE=1
		fi
		if [ "$var" = 'swig' ]; then
			echo "swig selected"
			CLEAR_SWIG=1
		fi
		if [ "$var" = 'pack' ]; then
			echo "pack selected"
			CLEAR_PACK=1
		fi
		if [ "$var" = 'coverage' ]; then
			echo "coverage selected"
			CLEAR_COVERAGE=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
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
	echo "	file"
	echo "	swig"
	echo "	pack"
	echo "	coverage"
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
