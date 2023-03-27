#!/usr/bin/env bash

#	MetaCall Configuration Environment Bash Script by Parra Studios
#   This script only installs python and nodejs for now.
#   It will be updated to install all the dependencies for all the languages.

INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_RUST=0
INSTALL_RAPIDJSON=0
INSTALL_FUNCHOOK=0
INSTALL_NETCORE=0
INSTALL_NETCORE2=0
INSTALL_NETCORE5=0
INSTALL_NETCORE7=0
INSTALL_V8=0
INSTALL_V8REPO=0
INSTALL_V8REPO58=0
INSTALL_V8REPO57=0
INSTALL_V8REPO54=0
INSTALL_V8REPO52=0
INSTALL_V8REPO51=0
INSTALL_NODEJS=0
INSTALL_TYPESCRIPT=0
INSTALL_FILE=0
INSTALL_RPC=0
INSTALL_WASM=0
INSTALL_JAVA=0
INSTALL_C=0
INSTALL_COBOL=0
INSTALL_SWIG=0
INSTALL_METACALL=0
INSTALL_PACK=0
INSTALL_COVERAGE=0
INSTALL_CLANGFORMAT=0
INSTALL_BACKTRACE=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Swig
sub_swig() {
    brew install swig
}

# Python
sub_python() {
    echo "configuring python"
	brew install pyenv openssl
	export PKG_CONFIG_PATH=$(brew --prefix openssl)/lib/pkgconfig
	export PYTHON_CONFIGURE_OPTS="--enable-shared"
	pyenv install 3.11.1
	pyenv global 3.11.1
	pyenv rehash
	echo -e '\nif command -v pyenv 1>/dev/null 2>&1; then\n  eval "$(pyenv init -)"\nfi' >> ~/.bash_profile
	source ~/.bash_profile
	which python3
	pip3 install requests
	pip3 install setuptools
	pip3 install wheel
	pip3 install rsa
	pip3 install scipy
	pip3 install numpy
	pip3 install joblib
	pip3 install scikit-learn
}

# NodeJS
sub_nodejs(){
	echo "configuring nodejs"
	brew install node make npm curl python3
}

# Java
sub_java(){
	echo "configuring java"
	brew install openjdk@19
}

sub_install()
{
	if [ $INSTALL_PYTHON = 1 ]; then
		sub_python
	fi
	if [ $INSTALL_NODEJS = 1 ]; then
		sub_nodejs
	fi
	if [ $INSTALL_JAVA = 1 ]; then
		sub_java
	fi
}

sub_options(){
	for var in "$@"
	do
		if [ "$var" = 'cache' ]; then
			echo "apt caching selected"
			APT_CACHE=1
		fi
		if [ "$var" = 'base' ]; then
			echo "apt selected"
			INSTALL_APT=1
		fi
		if [ "$var" = 'python' ]; then
			echo "python selected"
			INSTALL_PYTHON=1
		fi
		if [ "$var" = 'ruby' ]; then
			echo "ruby selected"
			INSTALL_RUBY=1
		fi
		if [ "$var" = 'rust' ]; then
			echo "rust selected"
			INSTALL_RUST=1
		fi
		if [ "$var" = 'netcore' ]; then
			echo "netcore selected"
			INSTALL_NETCORE=1
		fi
		if [ "$var" = 'netcore2' ]; then
			echo "netcore 2 selected"
			INSTALL_NETCORE2=1
		fi
		if [ "$var" = 'netcore5' ]; then
			echo "netcore 5 selected"
			INSTALL_NETCORE5=1
		fi
		if [ "$var" = 'netcore7' ]; then
			echo "netcore 7 selected"
			INSTALL_NETCORE7=1
		fi
		if [ "$var" = 'rapidjson' ]; then
			echo "rapidjson selected"
			INSTALL_RAPIDJSON=1
		fi
		if [ "$var" = 'funchook' ]; then
			echo "funchook selected"
			INSTALL_FUNCHOOK=1
		fi
		if [ "$var" = 'v8' ] || [ "$var" = 'v8rep54' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO54=1
		fi
		if [ "$var" = 'v8rep57' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO57=1
		fi
		if [ "$var" = 'v8rep58' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO58=1
		fi
		if [ "$var" = 'v8rep52' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO52=1
		fi
		if [ "$var" = 'v8rep51' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO51=1
		fi
		if [ "$var" = 'nodejs' ]; then
			echo "nodejs selected"
			INSTALL_NODEJS=1
		fi
		if [ "$var" = 'typescript' ]; then
			echo "typescript selected"
			INSTALL_TYPESCRIPT=1
		fi
		if [ "$var" = 'file' ]; then
			echo "file selected"
			INSTALL_FILE=1
		fi
		if [ "$var" = 'rpc' ]; then
			echo "rpc selected"
			INSTALL_RPC=1
		fi
		if [ "$var" = 'wasm' ]; then
			echo "wasm selected"
			INSTALL_WASM=1
		fi
		if [ "$var" = 'java' ]; then
			echo "java selected"
			INSTALL_JAVA=1
		fi
		if [ "$var" = 'c' ]; then
			echo "c selected"
			INSTALL_C=1
		fi
		if [ "$var" = 'cobol' ]; then
			echo "cobol selected"
			INSTALL_COBOL=1
		fi
		if [ "$var" = 'swig' ]; then
			echo "swig selected"
			INSTALL_SWIG=1
		fi
		if [ "$var" = 'metacall' ]; then
			echo "metacall selected"
			INSTALL_METACALL=1
		fi
		if [ "$var" = 'pack' ]; then
			echo "pack selected"
			INSTALL_PACK=1
		fi
		if [ "$var" = 'coverage' ]; then
			echo "coverage selected"
			INSTALL_COVERAGE=1
		fi
		if [ "$var" = 'clangformat' ]; then
			echo "clangformat selected"
			INSTALL_CLANGFORMAT=1
		fi
		if [ "$var" = 'backtrace' ]; then
			echo "backtrace selected"
			INSTALL_BACKTRACE=1
		fi
	done
}

case "$#" in
	0)
		sub_help
		;;
	*)
		sub_options $@
		sub_install
		;;
esac
