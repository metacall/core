#!/usr/bin/env bash

#
#	MetaCall Configuration Environment Bash Script by Parra Studios
#	Configure and install MetaCall environment script utility.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
APT_CACHE=0
APT_CACHE_CMD=""
INSTALL_APT=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_RAPIDJSON=0
INSTALL_FUNCHOOK=0
INSTALL_NETCORE=0
INSTALL_NETCORE2=0
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
INSTALL_WASM=0
INSTALL_SWIG=0
INSTALL_METACALL=0
INSTALL_PACK=0
INSTALL_COVERAGE=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Base packages
sub_apt(){
	echo "configure apt"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install build-essential git cmake wget apt-utils apt-transport-https gnupg dirmngr ca-certificates
}

# Swig
sub_swig(){
	echo "configure swig"
	cd $ROOT_DIR
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install g++ libpcre3-dev tar

	wget http://prdownloads.sourceforge.net/swig/swig-4.0.1.tar.gz

	tar -xzf swig-4.0.1.tar.gz
	cd swig-4.0.1
	./configure --prefix=/usr/local
	make
	$SUDO_CMD make install
	cd ..
	rm -rf swig-4.0.1

	# Install Python Port Dependencies (TODO: This must be transformed into pip3 install metacall)
	$SUDO_CMD pip3 install setuptools


}

# Python
sub_python(){
	echo "configure python"
	cd $ROOT_DIR
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install python3 python3-dev python3-pip
	$SUDO_CMD pip3 install django
	$SUDO_CMD pip3 install requests
	$SUDO_CMD pip3 install setuptools
	$SUDO_CMD pip3 install wheel
	$SUDO_CMD pip3 install rsa
	$SUDO_CMD pip3 install scipy
	$SUDO_CMD pip3 install numpy
	$SUDO_CMD pip3 install scikit-learn
	$SUDO_CMD pip3 install joblib
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR

	# TODO: Remove this when using ruby2.5 (not available yet because it fails on loading a script with a malloc error)
	$SUDO_CMD mv /etc/apt/sources.list /etc/apt/sources.list.backup
	$SUDO_CMD sh -c "echo \"deb http://ftp.debian.org/debian/ stretch main\" > /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb-src http://ftp.debian.org/debian/ stretch main\" >> /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb http://security.debian.org/debian-security stretch/updates main\" >> /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb-src http://security.debian.org/debian-security stretch/updates main\" >> /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb http://ftp.debian.org/debian/ stretch-updates main\" >> /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb-src http://ftp.debian.org/debian/ stretch-updates main\" >> /etc/apt/sources.list"

	$SUDO_CMD apt-get update
	#$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends --allow-remove-essential install git-core curl zlib1g-dev build-essential libssl-dev libreadline-dev libyaml-dev libsqlite3-dev sqlite3 libxml2-dev libxslt1-dev libcurl4-openssl-dev software-properties-common libffi-dev ruby2.5 libruby2.5 ruby2.5-dev
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends --allow-remove-essential --allow-downgrades install libgmp10=2:6.1.2+dfsg-1 libgmp-dev libncurses5 libtinfo5 ruby2.3 libruby2.3 ruby2.3-dev

	# TODO: Review conflict with NodeJS (currently rails test is disabled)
	#wget https://deb.nodesource.com/setup_4.x | $SUDO_CMD bash -
	#$SUDO_CMD apt-get -y --no-install-recommends install nodejs
	#$SUDO_CMD gem install rails

	# TODO: Remove this when using ruby2.5 (not available yet because it fails on loading a script with a malloc error)
	$SUDO_CMD mv /etc/apt/sources.list.backup /etc/apt/sources.list
}

# RapidJSON
sub_rapidjson(){
	echo "configure rapidjson"
	cd $ROOT_DIR
	git clone https://github.com/miloyip/rapidjson.git
	cd rapidjson
	git checkout v1.1.0
	mkdir build
	cd build
	cmake -DRAPIDJSON_BUILD_DOC=Off -DRAPIDJSON_BUILD_EXAMPLES=Off -DRAPIDJSON_BUILD_TESTS=Off ..
	make
	$SUDO_CMD make install
	cd ../.. && rm -rf ./rapidjson
}

# FuncHook
sub_funchook(){
	echo "configure funchook"
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install cmake
}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR

	# Debian Stretch

	$SUDO_CMD apt-get update && apt-get $APT_CACHE_CMD install -y --no-install-recommends \
		libc6 libcurl3 libgcc1 libgssapi-krb5-2 libicu57 liblttng-ust0 libssl1.0.2 libstdc++6 libunwind8 libuuid1 zlib1g

	# Install .NET Sdk
	DOTNET_SDK_VERSION=1.1.11
	DOTNET_SDK_DOWNLOAD_URL=https://dotnetcli.blob.core.windows.net/dotnet/Sdk/$DOTNET_SDK_VERSION/dotnet-dev-debian.9-x64.$DOTNET_SDK_VERSION.tar.gz

	wget $DOTNET_SDK_DOWNLOAD_URL -O dotnet.tar.gz
	mkdir -p /usr/share/dotnet
	tar -zxf dotnet.tar.gz -C /usr/share/dotnet
	rm dotnet.tar.gz
	ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet

	# Trigger the population of the local package cache
	mkdir warmup
	cd warmup
	dotnet new
	cd ..
	rm -rf warmup
	rm -rf /tmp/NuGetScratch
}

# NetCore 2
sub_netcore2(){
	echo "configure netcore 2"
	cd $ROOT_DIR

	# Set up repository
	wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
	$SUDO_CMD dpkg -i packages-microsoft-prod.deb
	rm packages-microsoft-prod.deb

	# Install .NET Core Sdk
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends apt-transport-https
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends dotnet-sdk-2.2
}

# V8 Repository
sub_v8repo(){
	echo "configure v8 from repository"
	cd $ROOT_DIR
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install software-properties-common

	# V8 5.1
	if [ $INSTALL_V8REPO51 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-archived/ubuntu trusty main\" > /etc/apt/sources.list.d/libv851.list"
		$SUDO_CMD sh -c "echo \"deb http://archive.ubuntu.com/ubuntu trusty main\" > /etc/apt/sources.list.d/libicu52.list"
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends --allow-unauthenticated install libicu52 libv8-5.1.117 libv8-5.1-dev
	fi

	# V8 5.4
	if [ $INSTALL_V8REPO54 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-5.4/ubuntu xenial main\" > /etc/apt/sources.list.d/libv854.list"
		wget http://launchpadlibrarian.net/234847357/libicu55_55.1-7_amd64.deb
		$SUDO_CMD dpkg -i libicu55_55.1-7_amd64.deb
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends --allow-unauthenticated install libicu55 libv8-5.4-dev
		$SUDO_CMD rm libicu55_55.1-7_amd64.deb
	fi

	# V8 5.2
	if [ $INSTALL_V8REPO52 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.2
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install libicu55 libv8-5.2-dev
	fi

	# V8 5.7
	if [ $INSTALL_V8REPO57 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.7
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install libicu55 libv8-5.7-dev
	fi

	# V8 5.8
	if [ $INSTALL_V8REPO58 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-archived/ubuntu trusty main\" > /etc/apt/sources.list.d/libv8-archived.list"
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install libicu57 libv8-5.8.283 libv8-5.8-dev
	fi
}

# V8
sub_v8(){
	echo "configure v8"
	cd $ROOT_DIR
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install python
	git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
	export PATH=`pwd`/depot_tools:"$PATH"

	export GYP_DEFINES="snapshot=on linux_use_bundled_gold=0 linux_use_gold_flags=0 component=shared_library"

	fetch v8
	cd v8
	git checkout 5.1-lkgr
	gclient sync

	patch build/all.gyp $ROOT_DIR/nobuildtest.patch
	GYP_DEFINES="snapshot=on linux_use_bundled_gold=0 linux_use_gold_flags=0 component=shared_library" make library=shared native
}

# NodeJS
sub_nodejs(){
	# TODO: Review conflicts with Ruby Rails and NodeJS 4.x
	echo "configure nodejs"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update

	# Install python 2.7 to build node (gyp)
	$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends install python g++ make nodejs curl

	# Install and update npm and node-gyp
	curl -L https://npmjs.org/install.sh | $SUDO_CMD sh
	npm i npm@latest -g
	npm i node-gyp@latest -g
}

# TypeScript
sub_typescript(){
	echo "configure typesecript"
}

# File
sub_file(){
	echo "configure file"
}

# WebAssembly
sub_wasm(){
	echo "configure webassembly"

	# TODO

	# $SUDO_CMD apt-get update
	# $SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends --fix-broken install lib32gcc-6-dev g++-multilib
}

# MetaCall
sub_metacall(){
	# TODO: Update this or deprecate it
	echo "configure metacall"
	cd $ROOT_DIR
	git clone --recursive https://github.com/metacall/core.git
	mkdir core/build && cd core/build

	if [ $INSTALL_NETCORE = 1 ]; then
		NETCORE_VERSION=1.1.10
	elif [ INSTALL_NETCORE2 = 1 ]; then
		NETCORE_VERSION=2.2.8
	else
		NETCORE_VERSION=0
	fi

	cmake -Wno-dev ../ -DPYTHON_EXECUTABLE=/usr/bin/python3.7 -DOPTION_BUILD_EXAMPLES=off -DOPTION_BUILD_LOADERS_PY=on -DOPTION_BUILD_LOADERS_RB=on -DOPTION_BUILD_LOADERS_CS=on -DOPTION_BUILD_LOADERS_JS=on -DCMAKE_BUILD_TYPE=Release -DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/$NETCORE_VERSION/
	make
	make test && echo "test ok!"

	echo "configure with cmake .. <options>"
}

# Pack
sub_pack(){
	echo "configure pack"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends rpm
}

# Coverage
sub_coverage(){
	echo "configure coverage"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get install -y --no-install-recommends lcov
}

# Install
sub_install(){
	if [ $RUN_AS_ROOT = 1 ]; then
		SUDO_CMD=""
	fi
	if [ $APT_CACHE = 1 ]; then
		APT_CACHE_CMD=-o dir::cache::archives="$APT_CACHE_DIR"
	fi
	if [ $INSTALL_APT = 1 ]; then
		sub_apt
	fi
	if [ $INSTALL_PYTHON = 1 ]; then
		sub_python
	fi
	if [ $INSTALL_RUBY = 1 ]; then
		sub_ruby
	fi
	if [ $INSTALL_RAPIDJSON = 1 ]; then
		sub_rapidjson
	fi
	if [ $INSTALL_FUNCHOOK = 1 ]; then
		sub_funchook
	fi
	if [ $INSTALL_NETCORE = 1 ]; then
		sub_netcore
	fi
	if [ $INSTALL_NETCORE2 = 1 ]; then
		sub_netcore2
	fi
	if [ $INSTALL_V8 = 1 ]; then
		sub_v8
	fi
	if [ $INSTALL_V8REPO = 1 ]; then
		sub_v8repo
	fi
	if [ $INSTALL_NODEJS = 1 ]; then
		sub_nodejs
	fi
	if [ $INSTALL_TYPESCRIPT = 1 ]; then
		sub_typescript
	fi
	if [ $INSTALL_FILE = 1 ]; then
		sub_file
	fi
	if [ $INSTALL_WASM = 1 ]; then
		sub_wasm
	fi
	if [ $INSTALL_SWIG = 1 ]; then
		sub_swig
	fi
	if [ $INSTALL_METACALL = 1 ]; then
		sub_metacall
	fi
	if [ $INSTALL_PACK = 1 ]; then
		sub_pack
	fi
	if [ $INSTALL_COVERAGE = 1 ]; then
		sub_coverage
	fi

	echo "install finished in workspace $ROOT_DIR"
}

# Configuration
sub_options(){
	for var in "$@"
	do
		if [ "$var" = 'root' ]; then
			echo "running as root"
			RUN_AS_ROOT=1
		fi
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
		if [ "$var" = 'netcore' ]; then
			echo "netcore selected"
			INSTALL_NETCORE=1
		fi
		if [ "$var" = 'netcore2' ]; then
			echo "netcore 2 selected"
			INSTALL_NETCORE2=1
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
		if [ "$var" = 'wasm' ]; then
			echo "wasm selected"
			INSTALL_WASM=1
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
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
	echo "Components:"
	echo "	root"
	echo "	cache"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	netcore2"
	echo "	rapidjson"
	echo "	funchook"
	echo "	v8"
	echo "	v8rep51"
	echo "	v8rep54"
	echo "	v8rep57"
	echo "	v8rep58"
	echo "	nodejs"
	echo "	typescript"
	echo "	file"
	echo "	wasm"
	echo "	swig"
	echo "	metacall"
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
		sub_install
		;;
esac
