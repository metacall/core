#!/bin/bash

#
#	MetaCall Configuration Environment Bash Script by Parra Studios
#	Configure and install MetaCall environment script utility.
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
INSTALL_APT=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_RAPIDJSON=0
INSTALL_FUNCHOOK=0
INSTALL_NETCORE=0
INSTALL_V8=0
INSTALL_V8REPO=0
INSTALL_V8REPO58=0
INSTALL_V8REPO57=0
INSTALL_V8REPO54=0
INSTALL_V8REPO52=0
INSTALL_V8REPO51=0
INSTALL_NODEJS=0
INSTALL_SWIG=0
INSTALL_METACALL=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Base packages
sub_apt(){
	echo "configure apt"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get -y --no-install-recommends install build-essential git cmake wget apt-utils apt-transport-https gnupg dirmngr ca-certificates
}

# Swig
sub_swig(){
	echo "configure swig"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y --no-install-recommends install libpcre3-dev
	wget "https://downloads.sourceforge.net/project/swig/swig/swig-3.0.12/swig-3.0.12.tar.gz?r=http%3A%2F%2Fwww.swig.org%2Fdownload.html&ts=1487810080&use_mirror=netix" -O swig.tar.gz
	mkdir swig
	tar -xf swig.tar.gz -C ./swig --strip-components=1
	cd swig
	./configure
	make
	$SUDO_CMD make install
	cd ..
	rm -rf ./swig swig.tar.gz
}

# Python
sub_python(){
	echo "configure python"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y --no-install-recommends install python3 python3-dev python3-pip
	$SUDO_CMD pip3 install django
	$SUDO_CMD pip3 install requests
	$SUDO_CMD pip3 install rsa
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get -y --no-install-recommends install git-core curl zlib1g-dev build-essential libssl-dev libreadline-dev libyaml-dev libsqlite3-dev sqlite3 libxml2-dev libxslt1-dev libcurl4-openssl-dev software-properties-common libffi-dev ruby2.3-dev

	# TODO: Review conflict with NodeJS (currently rails test is disabled)
	#curl -sL https://deb.nodesource.com/setup_4.x | $SUDO_CMD bash -
	#$SUDO_CMD apt-get -y --no-install-recommends install nodejs
	#$SUDO_CMD gem install rails
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
	cmake ..
	make
	$SUDO_CMD make install
	cd ../.. && rm -rf ./rapidjson
}

# FuncHook
sub_funchook(){
	echo "configure funchook"
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get -y --no-install-recommends install autoconf
}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR

	$SUDO_CMD apt-get update && apt-get install -y --no-install-recommends \
		libc6 libcurl3 libgcc1 libgssapi-krb5-2 libicu57 liblttng-ust0 libssl1.0.2 libstdc++6 libunwind8 libuuid1 zlib1g

	# # Install .NET Core
	# DOTNET_VERSION=1.1.10
	# DOTNET_DOWNLOAD_URL=https://dotnetcli.blob.core.windows.net/dotnet/Runtime/$DOTNET_VERSION/dotnet-debian.9-x64.$DOTNET_VERSION.tar.gz

	# wget $DOTNET_DOWNLOAD_URL -O dotnet.tar.gz
	# mkdir -p /usr/share/dotnet
	# tar -zxf dotnet.tar.gz -C /usr/share/dotnet
	# rm dotnet.tar.gz
	# ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet

	# Install .NET Runtime
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

# V8 Repository
sub_v8repo(){
	# TODO: Switch repositories from ubuntu to debian

	echo "configure v8 from repository"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y --no-install-recommends install add-apt-key
	$SUDO_CMD apt-get -y --no-install-recommends install software-properties-common

	# V8 5.1
	if [ $INSTALL_V8REPO51 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-archived/ubuntu trusty main\" > /etc/apt/sources.list.d/libv851.list"
		$SUDO_CMD sh -c "echo \"deb http://archive.ubuntu.com/ubuntu trusty main\" > /etc/apt/sources.list.d/libicu52.list"
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --no-install-recommends --allow-unauthenticated install libicu52 libv8-5.1.117 libv8-5.1-dev
	fi

	# V8 5.4
	if [ $INSTALL_V8REPO54 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-5.4/ubuntu xenial main\" > /etc/apt/sources.list.d/libv854.list"
		wget http://launchpadlibrarian.net/234847357/libicu55_55.1-7_amd64.deb
		$SUDO_CMD dpkg -i libicu55_55.1-7_amd64.deb
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --no-install-recommends --allow-unauthenticated install libicu55 libv8-5.4-dev
		$SUDO_CMD rm libicu55_55.1-7_amd64.deb
	fi

	# V8 5.2
	if [ $INSTALL_V8REPO52 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.2
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --no-install-recommends install libicu55 libv8-5.2-dev
	fi

	# V8 5.8
	if [ $INSTALL_V8REPO58 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.8
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --no-install-recommends install libicu55 libv8-5.8-dev
	fi

	# V8 5.7
	if [ $INSTALL_V8REPO57 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.7
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --no-install-recommends install libicu55 libv8-5.7-dev
	fi
}

# V8
sub_v8(){
	echo "configure v8"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y --no-install-recommends install python
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
	$SUDO_CMD apt-get -y --no-install-recommends install python build-essential libssl1.0.2 libssl1.0-dev

	# Install NodeJS from distributable (TODO: Keys not working)
	NODE_VERSION=8.11.1
	PACKAGE_SUFFIX=tar.xz

	# for key in \
	# 	94AE36675C464D64BAFA68DD7434390BDBE9B9C5 \
	# 	FD3A5288F042B6850C66B31F09FE44734EB7990E \
	# 	71DCFD284A79C3B38668286BC97EC7A07EDE3FC1 \
	# 	DD8F2338BAE7501E3DD5AC78C273792F7D83545D \
	# 	C4F0DFFF4E8C1A8236409D08E73BC641CC11F4C8 \
	# 	B9AE9905FFD7803F25714661B63B535A4C206CA9 \
	# 	56730D5401028683275BD23C23EFEFE93C4CFFFE \
	# 	77984A986EBC2AA786BC0F66B01FBB92821C587A \
	# 	8FCCA13FEF1D0C2E91008E09770F7A9A5AE15600; \
	# do
	# 	gpg --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys "$key" \
	# 	|| gpg --keyserver hkp://ipv4.pool.sks-keyservers.net --recv-keys "$key" \
	# 	|| gpg --keyserver hkp://pgp.mit.edu:80 --recv-keys "$key"
	# done

	DPKG_ARCH="$(dpkg --print-architecture)"

	case "${DPKG_ARCH##*-}" in
		amd64) ARCH='x64';;
		ppc64el) ARCH='ppc64le';;
		s390x) ARCH='s390x';;
		arm64) ARCH='arm64';;
		armhf) ARCH='armv7l';;
		i386) ARCH='x86';;
		*) echo "unsupported architecture ($DPKG_ARCH) for nodejs"; return 1;;
	esac

	wget --no-check-certificate "https://nodejs.org/dist/v$NODE_VERSION/node-v$NODE_VERSION-linux-$ARCH.$PACKAGE_SUFFIX"
	wget --no-check-certificate "https://nodejs.org/dist/v$NODE_VERSION/SHASUMS256.txt.asc"
	# gpg --batch --decrypt --output SHASUMS256.txt SHASUMS256.txt.asc
	# grep " node-v$NODE_VERSION-linux-$ARCH.tar.xz\$" SHASUMS256.txt | sha256sum -c -
	tar -xJf "node-v$NODE_VERSION-linux-$ARCH.tar.xz" -C /usr/local --strip-components=1 --no-same-owner
	rm "node-v$NODE_VERSION-linux-$ARCH.tar.xz" SHASUMS256.txt.asc # SHASUMS256.txt
	$SUDO_CMD ln -s /usr/local/bin/node /usr/local/bin/nodejs

	# Update npm and install node-gyp
	npm i npm@latest -g
	npm i node-gyp -g

	# # Install pkg config for icu library
	# $SUDO_CMD apt-get -y --no-install-recommends install pkg-config
	# pkg-config icu-i18n --cflags --libs
}

# MetaCall
sub_metacall(){
	echo "configure metacall"
	cd $ROOT_DIR
	git clone --recursive git@bitbucket.org:parrastudios/metacall.git
	cd metacall
	git checkout develop
	git submodule update --init
	mkdir build
	cd build

	cmake ../ -DPYTHON_EXECUTABLE=/usr/bin/python3.5 -DOPTION_BUILD_EXAMPLES=off -DOPTION_BUILD_PORTS=off -DOPTION_BUILD_PORTS_PY=off -DOPTION_BUILD_PORTS_JS=off -DOPTION_BUILD_PORTS_RB=off -DOPTION_BUILD_LOADERS_PY=on -DOPTION_BUILD_LOADERS_RB=on -DOPTION_BUILD_LOADERS_CS=on -DOPTION_BUILD_LOADERS_CS_IMPL=on -DOPTION_BUILD_LOADERS_JS=on -DCMAKE_BUILD_TYPE=Release -DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.10/
	make
	make test && echo "test ok!"

	echo "configure with cmake .. <options>"
}

# Install
sub_install(){
	if [ $RUN_AS_ROOT = 1 ]; then
		SUDO_CMD=""
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
	if [ $INSTALL_V8 = 1 ]; then
		sub_v8
	fi
	if [ $INSTALL_V8REPO = 1 ]; then
		sub_v8repo
	fi
	if [ $INSTALL_NODEJS = 1 ]; then
		sub_nodejs
	fi
	if [ $INSTALL_SWIG = 1 ]; then
		sub_swig
	fi
	if [ $INSTALL_METACALL = 1 ]; then
		sub_metacall
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
		if [ "$var" = 'swig' ]; then
			echo "swig selected"
			INSTALL_SWIG=1
		fi
		if [ "$var" = 'metacall' ]; then
			echo "metacall selected"
			INSTALL_METACALL=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: $PROGNAME list of component"
	echo "Components:"
	echo "	root"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	rapidjson"
	echo "	funchook"
	echo "	v8"
	echo "	v8rep51"
	echo "	v8rep54"
	echo "	v8rep57"
	echo "	v8rep58"
	echo "	nodejs"
	echo "	swig"
	echo "	metacall"
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
