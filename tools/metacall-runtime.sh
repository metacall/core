#!/usr/bin/env sh

#
#	MetaCall Configuration Environment Shell Script by Parra Studios
#	Configure and install MetaCall environment script utility.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

set -euxo

ROOT_DIR=$(pwd)

BUILD_TYPE=Release
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
INSTALL_SANDBOX=0
INSTALL_PORTS=0
INSTALL_CLEAN=0
SHOW_HELP=0
PROGNAME=$(basename $0)

# Operative System detection
case "$(uname -s)" in
	Linux*)		OPERATIVE_SYSTEM=Linux;;
	Darwin*)	OPERATIVE_SYSTEM=Darwin;;
	CYGWIN*)	OPERATIVE_SYSTEM=Cygwin;;
	MINGW*)		OPERATIVE_SYSTEM=MinGW;;
	*)			OPERATIVE_SYSTEM="Unknown"
esac

# Check out for sudo
if [ "`id -u`" = '0' ]; then
	SUDO_CMD=""
else
	SUDO_CMD=sudo
fi

# Linux Distro detection
if [ -f /etc/os-release ]; then # Either Debian or Ubuntu
	# Cat file | Get the ID field | Remove 'ID=' | Remove leading and trailing spaces | Remove quotes
	LINUX_DISTRO=$(cat /etc/os-release | grep "^ID=" | cut -f2- -d= | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr -d '"')
	# Cat file | Get the ID field | Remove 'ID=' | Remove leading and trailing spaces | Remove quotes
	LINUX_VERSION_ID=$(cat /etc/os-release | grep "^VERSION_ID=" | cut -f2- -d= | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr -d '"')
else
	# TODO: Implement more distros or better detection
	LINUX_DISTRO=unknown
	LINUX_VERSION_ID=unknown
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

	if [ "${BUILD_TYPE}" = "Debug" ]; then
		sub_apt_install_hold python3-dbg libpython3-dbg
	else
		sub_apt_install_hold python3
	fi
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR

	$SUDO_CMD apt-get update
	sub_apt_install_hold ruby libruby
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
	sub_apt_install_hold dotnet-runtime-7.0=7.0.5-1
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
	sub_apt_install_hold libnode-dev
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

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			UBUNTU_CODENAME=""
			CODENAME_FROM_ARGUMENTS=""

			# Obtain VERSION_CODENAME and UBUNTU_CODENAME (for Ubuntu and its derivatives)
			. /etc/os-release

			case ${LINUX_DISTRO} in
				debian)
					if [ "${VERSION:-}" = "unstable" ] || [ "${VERSION:-}" = "testing" ]; then
						CODENAME="unstable"
					else
						CODENAME="${VERSION_CODENAME}"
					fi
					;;
				*)
					# Ubuntu and its derivatives
					if [ -n "${UBUNTU_CODENAME}" ]; then
						CODENAME="${UBUNTU_CODENAME}"
					fi
					;;
			esac

			if [ "${CODENAME}" = "trixie" ] || [ "${CODENAME}" = "unstable" ]; then
				sub_apt_install_hold libcurl4t64
			else
				sub_apt_install_hold libcurl4
			fi
		fi
	fi
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

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		LLVM_VERSION_STRING=14

		if [ "${LINUX_DISTRO}" = "debian" ]; then
			UBUNTU_CODENAME=""
			CODENAME_FROM_ARGUMENTS=""

			# Obtain VERSION_CODENAME and UBUNTU_CODENAME (for Ubuntu and its derivatives)
			. /etc/os-release

			case ${LINUX_DISTRO} in
				debian)
					# For now bookworm || trixie == sid, change when trixie is released
					if [ "${VERSION:-}" = "unstable" ] || [ "${VERSION:-}" = "testing" ] || [ "${VERSION_CODENAME}" = "bookworm" ] || [ "${VERSION_CODENAME}" = "trixie" ]; then
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
					if [ -n "${UBUNTU_CODENAME}" ]; then
						CODENAME="${UBUNTU_CODENAME}"
						if [ -n "${CODENAME}" ]; then
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
		elif [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			sub_apt_install_hold libffi libclang-${LLVM_VERSION_STRING}
		fi

		# TODO: Implement Alpine and Darwin
	fi
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

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			UBUNTU_CODENAME=""
			CODENAME_FROM_ARGUMENTS=""

			# Obtain VERSION_CODENAME and UBUNTU_CODENAME (for Ubuntu and its derivatives)
			. /etc/os-release

			case ${LINUX_DISTRO} in
				debian)
					if [ "${VERSION:-}" = "unstable" ] || [ "${VERSION:-}" = "testing" ]; then
						CODENAME="unstable"
					else
						CODENAME="${VERSION_CODENAME}"
					fi
					;;
				*)
					# Ubuntu and its derivatives
					if [ -n "${UBUNTU_CODENAME}" ]; then
						CODENAME="${UBUNTU_CODENAME}"
					fi
					;;
			esac

			if [ "${CODENAME}" = "trixie" ] || [ "${CODENAME}" = "unstable" ]; then
				sub_apt_install_hold libdw1t64 libelf1t64
			else
				sub_apt_install_hold libdw1
			fi
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache binutils
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install dwarfutils
		brew install libelf
	fi
}

# Sandbox (this provides sandboxing features in Linux through BFS filters with libseccomp)
sub_sandbox(){
	echo "configure sandbox"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get install -y --no-install-recommends libseccomp
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache libseccomp
		fi
	fi
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
	if [ $INSTALL_SANDBOX = 1 ]; then
		sub_sandbox
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
	for option in "$@"
	do
		if [ "$option" = 'debug' ]; then
			echo "debug mode selected"
			BUILD_TYPE=Debug
		fi
		if [ "$option" = 'release' ] || [ "$option" = 'relwithdebinfo' ]; then
			echo "release mode selected"
			BUILD_TYPE=Release
		fi
		if [ "$option" = 'base' ]; then
			echo "apt selected"
			INSTALL_APT=1
		fi
		if [ "$option" = 'python' ]; then
			echo "python selected"
			INSTALL_PYTHON=1
		fi
		if [ "$option" = 'ruby' ]; then
			echo "ruby selected"
			INSTALL_RUBY=1
		fi
		if [ "$option" = 'netcore' ]; then
			echo "netcore selected"
			INSTALL_NETCORE=1
		fi
		if [ "$option" = 'netcore2' ]; then
			echo "netcore 2 selected"
			INSTALL_NETCORE2=1
		fi
		if [ "$option" = 'netcore5' ]; then
			echo "netcore 5 selected"
			INSTALL_NETCORE5=1
		fi
		if [ "$option" = 'v8' ]; then
			echo "v8 selected"
			INSTALL_V8=1
		fi
		if [ "$option" = 'nodejs' ]; then
			echo "nodejs selected"
			INSTALL_NODEJS=1
		fi
		if [ "$option" = 'typescript' ]; then
			echo "typescript selected"
			INSTALL_TYPESCRIPT=1
		fi
		if [ "$option" = 'file' ]; then
			echo "file selected"
			INSTALL_FILE=1
		fi
		if [ "$option" = 'rpc' ]; then
			echo "rpc selected"
			INSTALL_RPC=1
		fi
		if [ "$option" = 'wasm' ]; then
			echo "wasm selected"
			INSTALL_WASM=1
		fi
		if [ "$option" = 'java' ]; then
			echo "java selected"
			INSTALL_JAVA=1
		fi
		if [ "$option" = 'c' ]; then
			echo "c selected"
			INSTALL_C=1
		fi
		if [ "$option" = 'cobol' ]; then
			echo "cobol selected"
			INSTALL_COBOL=1
		fi
		if [ "$option" = 'backtrace' ]; then
			echo "backtrace selected"
			INSTALL_BACKTRACE=1
		fi
		if [ "$option" = 'sandbox' ]; then
			echo "sandbox selected"
			INSTALL_SANDBOX=1
		fi
		if [ "$option" = 'ports' ]; then
			echo "ports selected"
			INSTALL_PORTS=1
		fi
		if [ "$option" = 'clean' ]; then
			echo "clean selected"
			INSTALL_CLEAN=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
	echo "Components:"
	echo "	debug | release | relwithdebinfo"
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
	echo "	sandbox"
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
