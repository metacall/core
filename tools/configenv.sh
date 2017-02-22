#!/bin/bash
ROOT_DIR=$(pwd)

INSTALL_APT=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_RAPIDJSON=0
INSTALL_NETCORE=0
INSTALL_V8=0
INSTALL_V8REPO=0
INSTALL_METACALL=0
SHOW_HELP=0 
PROGNAME=$(basename $0)

#initial apt
sub_apt(){
	cd $ROOT_DIR
	echo "configure apt for C build"
	sudo apt-get -y install build-essential git cmake 
}

#python
sub_python(){
	echo "configure pyton"
	sudo apt-get install -y python2.7  python3.5-dev python python3-pip
	sudo pip3 install django
	sudo pip3 install request
}

#ruby
sub_ruby(){
	echo "configure ruby"
	sudo apt-get install -y ruby2.3-dev
}

#rapidjson
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
	sudo make install
}

#netcore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR
	sudo apt-get -y install apt-transport-https
	sudo sh -c 'echo "deb [arch=amd64] https://apt-mo.trafficmanager.net/repos/dotnet-release/ yakkety main" > /etc/apt/sources.list.d/dotnetdev.list'
	sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 417A0893
	sudo apt-get update

	sudo apt-get -y install dotnet-dev-1.0.0-preview2.1-003177
}

#v8
sub_v8repo(){
	echo "configure v8 from repository"
	cd $ROOT_DIR
	sudo apt-get -y install add-apt-key
	sudo apt-get -y install software-properties-common python-software-properties
	sudo add-apt-repository -y ppa:pinepain/libv8-5.8
	sudo apt-get update
	sudo apt-get -y install libv8-5.8-dev 
}

#v8
sub_v8(){
	echo "configure v8"
	cd $ROOT_DIR

	sudo apt-get install -y python
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

sub_metacall(){
	echo "configure metacall"
	cd $ROOT_DIR
	sudo apt-get -y install swig3.0
	git clone --recursive git@bitbucket.org:parrastudios/metacall.git
	cd metacall
	git checkout develop
	mkdir build
	cd build

	cmake ../ -DPYTHON_EXECUTABLE=/usr/bin/python3.5 -DOPTION_BUILD_EXAMPLES=off -DOPTION_BUILD_PORTS=off -DOPTION_BUILD_PORTS_PY=off -DOPTION_BUILD_PORTS_JS=off -DOPTION_BUILD_PORTS_RB=off -DOPTION_BUILD_PLUGINS_PY=on -DOPTION_BUILD_PLUGINS_RB=on -DOPTION_BUILD_PLUGINS_CS=on -DCORECLR_ROOT_REPOSITORY_PATH=$ROOT_DIR/coreclr/ -DOPTION_BUILD_PLUGINS_CS_IMPL=on -DOPTION_BUILD_PLUGINS_JS=on -DV8_HOME=/opt/libv8-5.8/ -DCMAKE_BUILD_TYPE=Release -DDOTNET_CORE_PATH=/usr/share/dotnet/shared/Microsoft.NETCore.App/1.1.0/
	make
	make test && echo "test ok!"

	echo "configure with cmake .. <options>"
}


sub_install(){
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
	if [ $INSTALL_METACALL = 1 ]; then
		sub_metacall
	fi
}

sub_config(){
	for var in "$@"
	do
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
		if [ "$var" = 'v8rep' ]; then
		    echo "v8 selected"
			INSTALL_V8REPO=1
		fi
		if [ "$var" = 'v8' ]; then
		    echo "v8 selected"
			INSTALL_V8=1
		fi
		if [ "$var" = 'metacall' ]; then
		    echo "metacall selected"
			INSTALL_METACALL=1
		fi
	done
}

sub_help(){
    echo "Usage: $PROGNAME list of component"
    echo "Components:"
    echo "	python"
    echo "	ruby"
    echo "	netcore"
    echo "	rapidjson"
    echo "	v8"
	echo "	v8rep"
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
