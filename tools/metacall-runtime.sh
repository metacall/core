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
INSTALL_NETCORE=0
INSTALL_V8=0
INSTALL_NODEJS=0
INSTALL_PORTS=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Install and mark packages to avoid autoremove
sub_apt_install_hold(){
	$SUDO_CMD apt-get -y install --no-install-recommends $@
	$SUDO_CMD apt-mark hold $@
}

# Base packages
sub_apt(){
	echo "configure apt"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update && apt-get -y install --no-install-recommends wget
}

# Python
sub_python(){
	echo "configure python"
	cd $ROOT_DIR
	sub_apt_install_hold python3 libpython3.5
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR
	sub_apt_install_hold ruby2.3 libruby2.3 \
		libgdbm3 libncurses5 libssl1.0.2 libyaml-0-2 rake ruby ruby-did-you-mean \
		ruby-minitest ruby-net-telnet ruby-power-assert ruby-test-unit \
		rubygems-integration ca-certificates
}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR
	sub_apt_install_hold libc6 libcurl3 libgcc1 libgssapi-krb5-2 libicu57 \
		liblttng-ust0 libssl1.0.2 libstdc++6 libunwind8 libuuid1 zlib1g ca-certificates

	# Install .NET Core
	DOTNET_VERSION=1.1.10
	DOTNET_DOWNLOAD_URL=https://dotnetcli.blob.core.windows.net/dotnet/Runtime/$DOTNET_VERSION/dotnet-debian.9-x64.$DOTNET_VERSION.tar.gz

	wget $DOTNET_DOWNLOAD_URL -O dotnet.tar.gz
	mkdir -p /usr/share/dotnet
	tar -zxf dotnet.tar.gz -C /usr/share/dotnet
	rm dotnet.tar.gz
	ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet
}

# V8
sub_v8(){
	echo "configure v8"
	# TODO
}

# NodeJS
sub_nodejs(){
	echo "configure node"
	# Nothing needed, node_modules are local to the path,
	# runtime is located in /usr/local/lib, and node builtins
	# are already compiled in the runtime
}

# Ports
sub_ports(){
	echo "configure ports"

	# TODO: Implement correctly the metacall distribution
	# $SUDO_CMD pip3 install metacall
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
	if [ $INSTALL_NETCORE = 1 ]; then
		sub_netcore
	fi
	if [ $INSTALL_V8 = 1 ]; then
		sub_v8
	fi
	if [ $INSTALL_NODEJS = 1 ]; then
		sub_nodejs
	fi
	if [ $INSTALL_PORTS = 1 ]; then
		sub_ports
	fi

	echo "install finished in workspace $ROOT_DIR"
}

# Clean dependencies
sub_clean(){
	$SUDO_CMD apt-get -y remove wget
	$SUDO_CMD apt-get -y autoclean
	$SUDO_CMD apt-get -y autoremove
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
		if [ "$var" = 'v8' ]; then
			echo "v8 selected"
			INSTALL_V8=1
		fi
		if [ "$var" = 'nodejs' ]; then
			echo "nodejs selected"
			INSTALL_NODEJS=1
		fi
		if [ "$var" = 'ports' ]; then
			echo "ports selected"
			INSTALL_PORTS=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
	echo "Components:"
	echo "	root"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	v8"
	echo "	nodejs"
	echo "	ports"
	echo ""
}

case "$#" in
	0)
		sub_help
		;;
	*)
		sub_options $@
		sub_install
		sub_clean
		;;
esac
