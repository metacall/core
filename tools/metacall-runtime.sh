#!/usr/bin/env bash

#
#	MetaCall Configuration Environment Bash Script by Parra Studios
#	Configure and install MetaCall environment script utility.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

set -euxo pipefail

ROOT_DIR=$(pwd)

INSTALL_APT=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
INSTALL_NETCORE=0
INSTALL_NETCORE2=0
INSTALL_NETCORE5=0
INSTALL_V8=0
INSTALL_NODEJS=0
INSTALL_TYPESCRIPT=0
INSTALL_FILE=0
INSTALL_RPC=0
INSTALL_WASM=0
INSTALL_JAVA=0
INSTALL_C=0
INSTALL_COBOL=0
INSTALL_BACKTRACE=0
INSTALL_PORTS=0
INSTALL_CLEAN=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Check out for sudo
if [ "`id -u`" = '0' ]; then
	SUDO_CMD=""
else
	SUDO_CMD=sudo
fi

# Linux Distro detection
if [ -f /etc/os-release ]; then # Either Debian or Ubuntu
	# Cat file | Get the ID field | Remove 'ID=' | Remove leading and trailing spaces
	LINUX_DISTRO=$(cat /etc/os-release | grep "^ID=" | cut -f2- -d= | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
else
	# TODO: Implement more distros or better detection
	LINUX_DISTRO=unknown
fi

# Install and mark packages to avoid autoremove
sub_apt_install_hold(){
	$SUDO_CMD apt-get -y install --no-install-recommends $@
	$SUDO_CMD apt-mark hold $@
}

# Base packages
sub_apt(){
	echo "configure apt"
	cd $ROOT_DIR
	$SUDO_CMD apt-get update && apt-get -y install --no-install-recommends wget gpg apt-transport-https
}

# Python
sub_python(){
	echo "configure python"
	cd $ROOT_DIR
	sub_apt_install_hold python3 libpython3.9 # libpython3.11
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR

	$SUDO_CMD apt-get update
	sub_apt_install_hold ruby2.7 libruby2.7 # ruby3.1 libruby3.1
}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR

	# Debian Stretch

	sub_apt_install_hold libc6 libcurl3 libgcc1 libgssapi-krb5-2 libicu57 \
		liblttng-ust0 libssl1.0.2 libstdc++6 libunwind8 libuuid1 zlib1g ca-certificates

	# Install .NET Core Runtime 1.x
	DOTNET_VERSION=1.1.10
	DOTNET_DOWNLOAD_URL=https://dotnetcli.blob.core.windows.net/dotnet/Runtime/$DOTNET_VERSION/dotnet-debian.9-x64.$DOTNET_VERSION.tar.gz

	wget $DOTNET_DOWNLOAD_URL -O dotnet.tar.gz
	mkdir -p /usr/share/dotnet
	tar -zxf dotnet.tar.gz -C /usr/share/dotnet
	rm dotnet.tar.gz
	ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet
}

# NetCore 2
sub_netcore2(){
	echo "configure netcore 2"
	cd $ROOT_DIR

	# Install NET Core Runtime 2.x
	wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
	$SUDO_CMD dpkg -i packages-microsoft-prod.deb
	rm packages-microsoft-prod.deb

	$SUDO_CMD apt-get update
	sub_apt_install_hold dotnet-runtime-2.2=2.2.8-1
}

# NetCore 5
sub_netcore5(){
	echo "configure netcore 5"
	cd $ROOT_DIR

	# Install NET Core Runtime 5.x
	wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
	$SUDO_CMD dpkg -i packages-microsoft-prod.deb
	rm packages-microsoft-prod.deb

	$SUDO_CMD apt-get update
	sub_apt_install_hold dotnet-runtime-5.0=5.0.17-1
}

# NetCore 7
sub_netcore7(){
	echo "configure netcore 7"
	cd $ROOT_DIR

	# Install NET Core Runtime 7.x
	wget https://packages.microsoft.com/config/debian/11/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
	$SUDO_CMD dpkg -i packages-microsoft-prod.deb
	rm packages-microsoft-prod.deb

	$SUDO_CMD apt-get update
	sub_apt_install_hold dotnet-runtime-7.0=7.0.3-1
}

# V8
sub_v8(){
	echo "configure v8"
	# TODO
}

# NodeJS
sub_nodejs(){
	echo "configure node"

	# Install NodeJS library
	sub_apt_install_hold libnode72 # libnode108
}

# TypeScript
sub_typescript(){
	echo "configure typescript"
	# Nothing needed, node_modules are local to the path,
	# runtime is located in /usr/local/lib, and node builtins
	# are already compiled in the runtime
}

# File
sub_file(){
	echo "configure file"
	# Nothing needed
}

# RPC
sub_rpc(){
	echo "configure rpc"

	sub_apt_install_hold libcurl4
}

# WebAssembly
sub_wasm(){
	echo "configure wasm"

	# TODO
}

# Java
sub_java(){
	echo "configure java"

	sub_apt_install_hold default-jre
}

# C
sub_c(){
	echo "configure c"

	LLVM_VERSION_STRING=11
	UBUNTU_CODENAME=""
	CODENAME_FROM_ARGUMENTS=""

	# Obtain VERSION_CODENAME and UBUNTU_CODENAME (for Ubuntu and its derivatives)
	source /etc/os-release

	case ${LINUX_DISTRO} in
		debian)
			if [[ "${VERSION}" == "unstable" ]] || [[ "${VERSION}" == "testing" ]]; then
				CODENAME="unstable"
				LINKNAME=""
			else
				# "stable" Debian release
				CODENAME="${VERSION_CODENAME}"
				LINKNAME="-${CODENAME}"
			fi
			;;
		*)
			# ubuntu and its derivatives
			if [[ -n "${UBUNTU_CODENAME}" ]]; then
				CODENAME="${UBUNTU_CODENAME}"
				if [[ -n "${CODENAME}" ]]; then
					LINKNAME="-${CODENAME}"
				fi
			fi
			;;
	esac

	wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO_CMD apt-key add
	$SUDO_CMD sh -c "echo \"deb http://apt.llvm.org/${CODENAME}/ llvm-toolchain${LINKNAME}-${LLVM_VERSION_STRING} main\" >> /etc/apt/sources.list"
	$SUDO_CMD sh -c "echo \"deb-src http://apt.llvm.org/${CODENAME}/ llvm-toolchain${LINKNAME}-${LLVM_VERSION_STRING} main\" >> /etc/apt/sources.list"
	$SUDO_CMD apt-get update
	sub_apt_install_hold libffi libclang-${LLVM_VERSION_STRING}
}

# Cobol
sub_cobol(){
	echo "configure cobol"

	if [ "${LINUX_DISTRO}" == "debian" ]; then
		echo "deb http://deb.debian.org/debian/ unstable main" | $SUDO_CMD tee -a /etc/apt/sources.list > /dev/null

		$SUDO_CMD apt-get update
		sub_apt_install_hold libcob4

		# Remove unstable from sources.list
		$SUDO_CMD head -n -2 /etc/apt/sources.list
		$SUDO_CMD apt-get update
	elif [ "${LINUX_DISTRO}" == "ubuntu" ]; then
		sub_apt_install_hold libcob4
	fi
}

# Backtrace (this only improves stack traces verbosity but backtracing is enabled by default)
sub_backtrace(){
	echo "configure backtrace"

	sub_apt_install_hold libdw1
}

# Ports
sub_ports(){
	echo "configure ports"

	# Nothing needed, there are no dependencies for ports by now
}

# Install
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
	if [ $INSTALL_NETCORE = 1 ]; then
		sub_netcore
	fi
	if [ $INSTALL_NETCORE2 = 1 ]; then
		sub_netcore2
	fi
	if [ $INSTALL_NETCORE5 = 1 ]; then
		sub_netcore5
	fi
	if [ $INSTALL_V8 = 1 ]; then
		sub_v8
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
	if [ $INSTALL_RPC = 1 ]; then
		sub_rpc
	fi
	if [ $INSTALL_WASM = 1 ]; then
		sub_wasm
	fi
	if [ $INSTALL_JAVA = 1 ]; then
		sub_java
	fi
	if [ $INSTALL_C = 1 ]; then
		sub_c
	fi
	if [ $INSTALL_COBOL = 1 ]; then
		sub_cobol
	fi
	if [ $INSTALL_BACKTRACE = 1 ]; then
		sub_backtrace
	fi
	if [ $INSTALL_PORTS = 1 ]; then
		sub_ports
	fi
	if [ $INSTALL_CLEAN = 1 ]; then
		sub_clean
	fi

	echo "install finished in workspace $ROOT_DIR"
}

# Clean dependencies
sub_clean(){
	echo "clean dependencies"

	$SUDO_CMD apt-get -y remove wget gpg
	$SUDO_CMD apt-get -y autoclean
}

# Configuration
sub_options(){
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
		if [ "$var" = 'netcore2' ]; then
			echo "netcore 2 selected"
			INSTALL_NETCORE2=1
		fi
		if [ "$var" = 'netcore5' ]; then
			echo "netcore 5 selected"
			INSTALL_NETCORE5=1
		fi
		if [ "$var" = 'v8' ]; then
			echo "v8 selected"
			INSTALL_V8=1
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
		if [ "$var" = 'backtrace' ]; then
			echo "backtrace selected"
			INSTALL_BACKTRACE=1
		fi
		if [ "$var" = 'ports' ]; then
			echo "ports selected"
			INSTALL_PORTS=1
		fi
		if [ "$var" = 'clean' ]; then
			echo "clean selected"
			INSTALL_CLEAN=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
	echo "Components:"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	netcore2"
	echo "	v8"
	echo "	nodejs"
	echo "	typescript"
	echo "	file"
	echo "	rpc"
	echo "	wasm"
	echo "	java"
	echo "	c"
	echo "	cobol"
	echo "	backtrace"
	echo "	ports"
	echo "	clean"
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
