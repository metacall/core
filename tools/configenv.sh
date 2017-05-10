#!/bin/bash

ROOT_DIR=$(pwd)

RUN_AS_ROOT=0
SUDO_CMD=sudo
INSTALL_APT=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_RAPIDJSON=0
INSTALL_NETCORE=0
INSTALL_V8=0
INSTALL_V8REPO=0
INSTALL_V8REPO58=0
INSTALL_V8REPO57=0
INSTALL_V8REPO54=0
INSTALL_V8REPO51=0
INSTALL_SWIG=0
INSTALL_METACALL=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Base packages
sub_apt(){
	cd $ROOT_DIR
	echo "configure apt for C build"
	$SUDO_CMD apt-get -y install build-essential git cmake wget
}

# Swig
sub_swig(){
	echo "configure swig"
	cd $ROOT_DIR
	sub_python
	sub_ruby
	$SUDO_CMD apt-get -y install libpcre3-dev
	wget "https://downloads.sourceforge.net/project/swig/swig/swig-3.0.12/swig-3.0.12.tar.gz?r=http%3A%2F%2Fwww.swig.org%2Fdownload.html&ts=1487810080&use_mirror=netix" -O swig.tar.gz
	mkdir swig
	tar -xf swig.tar.gz -C ./swig --strip-components=1
	cd swig
	./configure
	make
	$SUDO_CMD make install
}

# Python
sub_python(){
	echo "configure pyton"
	$SUDO_CMD apt-get install -y python2.7  python3.5-dev python python3-pip
	$SUDO_CMD pip3 install django
	$SUDO_CMD pip3 install request
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	$SUDO_CMD apt-get update
	$SUDO_CMD apt-get install -y git-core curl zlib1g-dev build-essential libssl-dev libreadline-dev libyaml-dev libsqlite3-dev sqlite3 libxml2-dev libxslt1-dev libcurl4-openssl-dev software-properties-common libffi-dev
	$SUDO_CMD apt-get install -y ruby2.3-dev
	curl -sL https://deb.nodesource.com/setup_4.x | $SUDO_CMD bash -
	$SUDO_CMD apt-get install -y nodejs
	$SUDO_CMD gem install rails
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
}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y install apt-transport-https libunwind8 libunwind8-dev gettext libicu-dev liblttng-ust-dev libcurl4-openssl-dev libssl-dev uuid-dev unzip
	$SUDO_CMD sh -c 'echo "deb [arch=amd64] https://apt-mo.trafficmanager.net/repos/dotnet-release/ yakkety main" > /etc/apt/sources.list.d/dotnetdev.list'
	$SUDO_CMD apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 417A0893
	$SUDO_CMD apt-get update

	$SUDO_CMD apt-get -y install libssl1.0.0 dotnet-sharedframework-microsoft.netcore.app-1.1.0 dotnet-dev-1.0.0-preview2.1-003177
}

# V8 Repository
sub_v8repo(){
	echo "configure v8 from repository"
	cd $ROOT_DIR
	$SUDO_CMD apt-get -y install add-apt-key
	$SUDO_CMD apt-get -y install software-properties-common

	# V8 5.1
	if [ $INSTALL_V8REPO51 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-5.1/ubuntu wily main\" > /etc/apt/sources.list.d/libv851.list"
		wget http://launchpadlibrarian.net/234847357/libicu55_55.1-7_amd64.deb
		$SUDO_CMD dpkg -i libicu55_55.1-7_amd64.deb
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --allow-unauthenticated install libv8-5.1-dev
	fi

	# V8 5.4
	if [ $INSTALL_V8REPO54 = 1 ]; then
		$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-5.4/ubuntu xenial main\" > /etc/apt/sources.list.d/libv854.list"
		wget http://launchpadlibrarian.net/234847357/libicu55_55.1-7_amd64.deb
		$SUDO_CMD dpkg -i libicu55_55.1-7_amd64.deb
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y --allow-unauthenticated install libv8-5.4-dev
	fi

	# V8 5.8
	if [ $INSTALL_V8REPO58 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.8
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y install libv8-5.8-dev
	fi

	# V8 5.7
	if [ $INSTALL_V8REPO57 = 1 ]; then
		$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.7
		$SUDO_CMD apt-get update
		$SUDO_CMD apt-get -y install libv8-5.7-dev
	fi
}

# V8
sub_v8(){
	echo "configure v8"
	cd $ROOT_DIR

	$SUDO_CMD apt-get install -y python
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

	cmake ../ -DPYTHON_EXECUTABLE=/usr/bin/python3.5 -DOPTION_BUILD_EXAMPLES=off -DOPTION_BUILD_PORTS=off -DOPTION_BUILD_PORTS_PY=off -DOPTION_BUILD_PORTS_JS=off -DOPTION_BUILD_PORTS_RB=off -DOPTION_BUILD_PLUGINS_PY=on -DOPTION_BUILD_PLUGINS_RB=on -DOPTION_BUILD_PLUGINS_CS=on -DCORECLR_ROOT_REPOSITORY_PATH=$ROOT_DIR/coreclr/ -DOPTION_BUILD_PLUGINS_CS_IMPL=on -DOPTION_BUILD_PLUGINS_JS=on -DCMAKE_BUILD_TYPE=Release -DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.0/
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
	if [ $INSTALL_NETCORE = 1 ]; then
		sub_netcore
	fi
	if [ $INSTALL_V8 = 1 ]; then
		sub_v8
	fi
	if [ $INSTALL_V8REPO = 1 ]; then
		sub_v8repo
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
sub_config(){
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

		if [ "$var" = 'v8rep54' ]; then
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
		if [ "$var" = 'v8rep51' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO51=1
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
sub_help(){
    echo "Usage: $PROGNAME list of component"
    echo "Components:"
    echo "	root"
    echo "	base"
    echo "	python"
    echo "	ruby"
    echo "	netcore"
    echo "	rapidjson"
    echo "	v8"
	echo "	v8rep51"
	echo "	v8rep54"
	echo "	v8rep57"
	echo "	v8rep58"
	echo "	swig"
    echo "	metacall"
	echo ""
}

case "$#" in
    0)
        sub_help
        ;;
    *)
		sub_config $@
		sub_install
        ;;
esac
