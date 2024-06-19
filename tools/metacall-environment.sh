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

APT_CACHE=0
APT_CACHE_CMD=""
BUILD_TYPE=Release
INSTALL_BASE=1
INSTALL_PYTHON=0
INSTALL_RUBY=0
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
INSTALL_GO=0
INSTALL_RUST=0
INSTALL_SWIG=0
INSTALL_PACK=0
INSTALL_COVERAGE=0
INSTALL_CLANGFORMAT=0
INSTALL_BACKTRACE=0
INSTALL_SANDBOX=0
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

# Base packages
sub_base(){
	echo "configure base packages"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends build-essential git cmake libgtest-dev wget apt-utils apt-transport-https gnupg dirmngr ca-certificates
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk update
			$SUDO_CMD apk add --no-cache g++ make git cmake gtest-dev wget gnupg ca-certificates
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install llvm cmake git googletest wget gnupg ca-certificates
	fi
}

# Swig
sub_swig(){
	echo "configure swig"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends g++ libpcre3-dev tar

			# Install Python Port Dependencies (TODO: This must be transformed into pip3 install metacall)
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends python3-setuptools
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache g++ pcre-dev tar

			# Install Python Port Dependencies (TODO: This must be transformed into pip3 install metacall)
			$SUDO_CMD apk add --no-cache py3-setuptools
		fi

		wget http://prdownloads.sourceforge.net/swig/swig-4.0.1.tar.gz

		tar -xzf swig-4.0.1.tar.gz
		cd swig-4.0.1
		./configure --prefix=/usr/local
		make -j$(getconf _NPROCESSORS_ONLN)
		$SUDO_CMD make install
		cd ..
		rm -rf swig-4.0.1

	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install swig
	fi
}

# Python
sub_python(){
	echo "configure python"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			if [ "${BUILD_TYPE}" = "Debug" ]; then
				PYTHON3_PKG=python3-dbg
			else
				PYTHON3_PKG=python3
			fi

			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends $PYTHON3_PKG python3-dev python3-pip

			# Python test dependencies
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends \
				python3-requests \
				python3-setuptools \
				python3-wheel \
				python3-rsa \
				python3-scipy \
				python3-numpy \
				python3-sklearn \
				python3-joblib

		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			# Fix to a lower Python version (3.9) in order avoid conflicts with Python dependency of Clang from C Loader
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.15/main python3=3.9.16-r0 python3-dev=3.9.16-r0

			# Python test dependencies
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.15/community \
				py3-pip=20.3.4-r1 \
				py3-rsa=4.7.2-r0 \
				py3-scipy=1.7.2-r0 \
				py3-numpy=1.21.4-r0 \
				py3-scikit-learn=0.24.0-r1 \
				py3-joblib=1.0.1-r1

			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.15/main \
				py3-requests=2.26.0-r1 \
				py3-setuptools=52.0.0-r4 \
				py3-wheel=0.36.2-r2
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install pyenv openssl
		export PKG_CONFIG_PATH=$(brew --prefix openssl)/lib/pkgconfig
		export PYTHON_CONFIGURE_OPTS="--enable-shared"
		pyenv install 3.11.1
		pyenv global 3.11.1
		pyenv rehash
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		ENV_FILE="$ROOT_DIR/build/.env"

		echo eval "$(pyenv init -)" >> $ENV_FILE
		. $ENV_FILE

		echo "-DPython3_INCLUDE_DIRS=$HOME/.pyenv/versions/3.11.1/include/python3.11" >> $CMAKE_CONFIG_PATH
		echo "-DPython3_LIBRARY=$HOME/.pyenv/versions/3.11.1/lib/libpython3.11.dylib" >> $CMAKE_CONFIG_PATH
		echo "-DPython3_EXECUTABLE=$HOME/.pyenv/versions/3.11.1/bin/python3.11" >> $CMAKE_CONFIG_PATH
		echo "-DPython3_ROOT=$HOME/.pyenv/versions/3.11.1" >> $CMAKE_CONFIG_PATH
		echo "-DPython3_VERSION=3.11.1" >> $CMAKE_CONFIG_PATH
		echo "-DPython3_FIND_FRAMEWORK=NEVER" >> $CMAKE_CONFIG_PATH

		pip3 install requests
		pip3 install setuptools
		pip3 install wheel
		pip3 install rsa
		pip3 install scipy
		pip3 install numpy
		pip3 install joblib
		pip3 install scikit-learn
	fi
}

# Ruby
sub_ruby(){
	echo "configure ruby"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends ruby ruby-dev

			# TODO: Review conflict with NodeJS (currently rails test is disabled)
			#wget https://deb.nodesource.com/setup_4.x | $SUDO_CMD bash -
			#$SUDO_CMD apt-get -y --no-install-recommends install nodejs
			#$SUDO_CMD gem install rails
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache ruby ruby-dev
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install ruby@3.2
		brew link ruby@3.2 --force --overwrite
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		RUBY_PREFIX="$(brew --prefix ruby@3.2)"
		RUBY_VERSION="$(ruby -e 'puts RUBY_VERSION')"
		echo "-DRuby_INCLUDE_DIR=$RUBY_PREFIX/include/ruby-3.2.0" >> $CMAKE_CONFIG_PATH
		echo "-DRuby_LIBRARY=$RUBY_PREFIX/lib/libruby.3.2.dylib" >> $CMAKE_CONFIG_PATH
		echo "-DRuby_EXECUTABLE=$RUBY_PREFIX/bin/ruby" >> $CMAKE_CONFIG_PATH
		echo "-DRuby_VERSION=$RUBY_VERSION" >> $CMAKE_CONFIG_PATH
	fi
}

# RapidJSON
sub_rapidjson(){
	echo "configure rapidjson"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		git clone https://github.com/miloyip/rapidjson.git
		cd rapidjson
		git checkout v1.1.0
		mkdir build
		cd build
		cmake -DRAPIDJSON_BUILD_DOC=Off -DRAPIDJSON_BUILD_EXAMPLES=Off -DRAPIDJSON_BUILD_TESTS=Off ..
		make -j$(getconf _NPROCESSORS_ONLN)
		$SUDO_CMD make install
		cd ../.. && rm -rf ./rapidjson
	fi
}

# FuncHook
sub_funchook(){
	echo "configure funchook"

}

# NetCore
sub_netcore(){
	echo "configure netcore"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			# Debian Stretch
			$SUDO_CMD apt-get update && $SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends \
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
		fi
	fi
}

# NetCore 2
sub_netcore2(){
	echo "configure netcore 2"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			# Set up repository
			wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
			$SUDO_CMD dpkg -i packages-microsoft-prod.deb
			rm packages-microsoft-prod.deb

			# Install .NET Core Sdk
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends apt-transport-https
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends dotnet-sdk-2.2
		fi
	fi
}

# NetCore 5
sub_netcore5(){
	echo "configure netcore 5"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			# Set up repository
			wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
			$SUDO_CMD dpkg -i packages-microsoft-prod.deb
			rm packages-microsoft-prod.deb

			# Install .NET Core Sdk
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends apt-transport-https
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends dotnet-sdk-5.0
		fi
	fi
}

# NetCore 7
sub_netcore7(){
	echo "configure netcore 7"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ]; then
			# Set up repository
			wget https://packages.microsoft.com/config/debian/11/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
			$SUDO_CMD dpkg -i packages-microsoft-prod.deb
			rm packages-microsoft-prod.deb

			# Install .NET Core Sdk
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends apt-transport-https
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends dotnet-sdk-7.0
		elif [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends dotnet-sdk-7.0
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache dotnet7-sdk
		fi
	fi
}

# V8 Repository
sub_v8repo(){
	echo "configure v8 from repository"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends software-properties-common

		# V8 5.1
		if [ $INSTALL_V8REPO51 = 1 ]; then
			$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-archived/ubuntu trusty main\" > /etc/apt/sources.list.d/libv851.list"
			$SUDO_CMD sh -c "echo \"deb http://archive.ubuntu.com/ubuntu trusty main\" > /etc/apt/sources.list.d/libicu52.list"
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends --allow-unauthenticated libicu52 libv8-5.1.117 libv8-5.1-dev
		fi

		# V8 5.4
		if [ $INSTALL_V8REPO54 = 1 ]; then
			$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-5.4/ubuntu xenial main\" > /etc/apt/sources.list.d/libv854.list"
			wget http://launchpadlibrarian.net/234847357/libicu55_55.1-7_amd64.deb
			$SUDO_CMD dpkg -i libicu55_55.1-7_amd64.deb
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends --allow-unauthenticated libicu55 libv8-5.4-dev
			$SUDO_CMD rm libicu55_55.1-7_amd64.deb
		fi

		# V8 5.2
		if [ $INSTALL_V8REPO52 = 1 ]; then
			$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.2
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends libicu55 libv8-5.2-dev
		fi

		# V8 5.7
		if [ $INSTALL_V8REPO57 = 1 ]; then
			$SUDO_CMD add-apt-repository -y ppa:pinepain/libv8-5.7
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends libicu55 libv8-5.7-dev
		fi

		# V8 5.8
		if [ $INSTALL_V8REPO58 = 1 ]; then
			$SUDO_CMD sh -c "echo \"deb http://ppa.launchpad.net/pinepain/libv8-archived/ubuntu trusty main\" > /etc/apt/sources.list.d/libv8-archived.list"
			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD -y --no-install-recommends libicu57 libv8-5.8.283 libv8-5.8-dev
		fi
	fi
}

# V8
sub_v8(){
	echo "configure v8"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends python
		git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
		export PATH=`pwd`/depot_tools:"$PATH"

		export GYP_DEFINES="snapshot=on linux_use_bundled_gold=0 linux_use_gold_flags=0 component=shared_library"

		fetch v8
		cd v8
		git checkout 5.1-lkgr
		gclient sync

		patch build/all.gyp $ROOT_DIR/nobuildtest.patch
		GYP_DEFINES="snapshot=on linux_use_bundled_gold=0 linux_use_gold_flags=0 component=shared_library" make -j$(getconf _NPROCESSORS_ONLN) library=shared native
	fi
}

# NodeJS
sub_nodejs(){
	echo "configure nodejs"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			# Note that Python is required for GYP
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends python3 g++ make nodejs npm curl
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache python3 g++ make nodejs nodejs-dev npm curl

			# Build dependencies (note libexecinfo-dev is not available in Alpine 3.17)
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.16/main linux-headers libexecinfo libexecinfo-dev

			# Build NodeJS shared library
			$SUDO_CMD apk add --no-cache --virtual .build-nodejs-deps \
				git \
				alpine-sdk \
				alpine-conf \
				ccache \
				brotli-dev \
				c-ares-dev \
				icu-dev \
				linux-headers \
				nghttp2-dev \
				openssl-dev \
				samurai \
				zlib-dev

			# Fix to a lower Python version (3.9) in order avoid conflicts with Python dependency of Clang from C Loader
			$SUDO_CMD apk add --no-cache --virtual .build-nodejs-python-deps --repository=https://dl-cdn.alpinelinux.org/alpine/v3.15/main \
				python3=3.9.16-r0 \
				py3-jinja2=3.0.1-r0

			git clone --depth 1 --branch v3.17.3 https://git.alpinelinux.org/aports
			cd aports/main/nodejs
			sed -i 's/--shared-brotli\ \\/--shared \\\n		--shared-brotli\ \\/g' APKBUILD
			if [ "$SUDO_CMD" = "" ]; then
				ABUILD_ROOT=-F
			else
				ABUILD_ROOT=
			fi
			echo "abuild-key" | abuild-keygen -a
			$SUDO_CMD cp abuild-key.pub /etc/apk/keys
			$SUDO_CMD abuild $ABUILD_ROOT checksum
			$SUDO_CMD abuild $ABUILD_ROOT || true
			cp pkg/nodejs/usr/bin/node /usr/bin/node
			cp src/node-v18.14.2/out/Release/lib/libnode.so.108 /usr/lib/.
			cd ../../..
			rm -rf aports
			$SUDO_CMD apk del .build-nodejs-deps
			$SUDO_CMD apk del .build-nodejs-python-deps
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		# TODO: Fork  https://github.com/puerts/backend-nodejs or let metacall build system compile NodeJS library itself
		brew install node@20
		# Make node 20 the default
		brew link node@20 --force --overwrite
		# Execute post install scripts
		brew postinstall node@20
		# Define node location
		NODE_PREFIX=$(brew --prefix node@20)
		# Include binaries into PATH
		export PATH="$NODE_PREFIX/bin:$PATH"

		# Configure NodeJS paths
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		echo "-DNodeJS_EXECUTABLE=$NODE_PREFIX/bin/node" >> $CMAKE_CONFIG_PATH
		# echo "-DNodeJS_INCLUDE_DIR=$NODE_PREFIX/include/node" >> $CMAKE_CONFIG_PATH
		# echo "-DNodeJS_LIBRARY=$NODE_PREFIX/lib/libnode.93.dylib" >> $CMAKE_CONFIG_PATH

		# Configure NPM path
		echo "-DNPM_ROOT=$NODE_PREFIX/bin" >> $CMAKE_CONFIG_PATH
	fi
}

# TypeScript
sub_typescript(){
	echo "configure typescript"

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		# Install React dependencies in order to run the tests
		$SUDO_CMD npm i react@latest -g
		$SUDO_CMD npm i react-dom@latest -g
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		# Install React dependencies in order to run the tests
		npm i react@latest -g
		npm i react-dom@latest -g
	fi
}

# File
sub_file(){
	echo "configure file"
}

# RPC
sub_rpc(){
	echo "cofingure rpc"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			# Install development files and documentation for libcurl (OpenSSL flavour)
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends libcurl4-openssl-dev
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache curl-dev
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install curl
	fi
}

# WebAssembly
sub_wasm(){
	echo "configure webassembly"

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/edge/testing wasmtime libwasmtime
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install wasmtime
	fi
}

# Java
sub_java(){
	echo "configure java"

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends default-jdk default-jre
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache openjdk8 openjdk8-jre
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install openjdk@17
		sudo ln -sfn $(brew --prefix openjdk@17)/libexec/openjdk.jdk /Library/Java/JavaVirtualMachines/openjdk-17.jdk
		JAVA_PREFIX=$(/usr/libexec/java_home -v 17)
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		echo "-DJAVA_HOME=$JAVA_PREFIX" >> $CMAKE_CONFIG_PATH
		echo "-DJAVA_INCLUDE_PATH=$JAVA_PREFIX/include" >> $CMAKE_CONFIG_PATH
		echo "-DJAVA_INCLUDE_PATH2=$JAVA_PREFIX/include/darwin" >> $CMAKE_CONFIG_PATH
		echo "-DJAVA_AWT_INCLUDE_PATH=$JAVA_PREFIX/include" >> $CMAKE_CONFIG_PATH
	fi
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
					# Ubuntu and its derivatives
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
			$SUDO_CMD apt-get install -y --no-install-recommends libffi-dev libclang-${LLVM_VERSION_STRING}-dev
		elif [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get install -y --no-install-recommends libffi-dev libclang-${LLVM_VERSION_STRING}-dev
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache libffi-dev
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/edge/testing tcc
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.16/main clang-libs=13.0.1-r1 clang-dev=13.0.1-r1
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install libffi
		brew install llvm@$LLVM_VERSION_STRING
		brew link llvm@$LLVM_VERSION_STRING --force --overwrite
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		LIBCLANG_PREFIX=$(brew --prefix llvm@$LLVM_VERSION_STRING)
		echo "-DLibClang_INCLUDE_DIR=${LIBCLANG_PREFIX}/include" >> $CMAKE_CONFIG_PATH
		echo "-DLibClang_LIBRARY=${LIBCLANG_PREFIX}/lib/libclang.dylib" >> $CMAKE_CONFIG_PATH
	fi
}

# Cobol
sub_cobol(){
	echo "configure cobol"

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ]; then
			echo "deb http://deb.debian.org/debian/ unstable main" | $SUDO_CMD tee -a /etc/apt/sources.list > /dev/null

			$SUDO_CMD apt-get update
			$SUDO_CMD apt-get $APT_CACHE_CMD -t unstable install -y --no-install-recommends gnucobol

			# Remove unstable from sources.list
			$SUDO_CMD head -n -2 /etc/apt/sources.list
		elif [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends gnucobol4
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache --virtual .build-cobol-deps build-base tar libaio libnsl libc6-compat binutils abuild make gcc gmp-dev db-dev libxml2-dev ncurses-dev

			# Install gnucobol
			wget https://sourceforge.net/projects/gnucobol/files/gnucobol/3.1/gnucobol-3.1-rc1.tar.gz/download -O gnucobol-3.1-rc1.tar.gz
			tar xvfz gnucobol-3.1-rc1.tar.gz
			cd gnucobol-3.1-rc1
			./configure
			make -j$(getconf _NPROCESSORS_ONLN)
			make install
			cd ..
			rm -rf gnucobol-3.1-rc1 gnucobol-3.1-rc1.tar.gz

			# Clean build deps
			$SUDO_CMD apk del .build-cobol-deps

			# Runtime deps
			$SUDO_CMD apk add --no-cache db ncurses
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install gnucobol
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		COBOL_PREFIX=$(brew --prefix gnucobol)
		echo "-DCOBOL_EXECUTABLE=${COBOL_PREFIX}/bin/cobc" >> $CMAKE_CONFIG_PATH
		echo "-DCOBOL_INCLUDE_DIR=${COBOL_PREFIX}/include" >> $CMAKE_CONFIG_PATH
		echo "-DCOBOL_LIBRARY=${COBOL_PREFIX}/lib/libcob.dylib" >> $CMAKE_CONFIG_PATH
	fi
}

# Go
sub_go(){
	echo "configure go"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends golang
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache go
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install go
	fi
}

# Rust
sub_rust(){
	echo "configure rust"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends curl autoconf automake
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache curl musl-dev linux-headers libgcc
		fi
		curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain nightly-2021-12-04 --profile default
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain nightly-2021-12-04 --profile default
		brew install patchelf
	fi
}

# Pack
sub_pack(){
	echo "configure pack"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get $APT_CACHE_CMD install -y --no-install-recommends rpm
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache rpm
		fi
	fi
}

# Coverage
sub_coverage(){
	echo "configure coverage"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get install -y --no-install-recommends lcov python3 python3-pip
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache lcov python3 py3-pip
		fi

		PIP_BREAK_SYSTEM_PACKAGES=`python3 -c 'import sys; sys.version_info.major >= 3 and sys.version_info.minor >= 11 and print("--break-system-packages", end="")'`

		pip3 install ${PIP_BREAK_SYSTEM_PACKAGES} gcovr==7.2
	fi
}

# Clang format
sub_clangformat(){
	echo "configure clangformat"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			LLVM_VERSION_STRING=12
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
			$SUDO_CMD apt-get install -y --no-install-recommends clang-format-${LLVM_VERSION_STRING}
			$SUDO_CMD ln -s /usr/bin/clang-format-${LLVM_VERSION_STRING} /usr/bin/clang-format
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/v3.15/main clang-extra-tools=12.0.1-r1
		fi
	fi
}

# Backtrace (this only improves stack traces verbosity but backtracing is enabled by default)
sub_backtrace(){
	echo "configure backtrace"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get install -y --no-install-recommends libdw-dev
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache binutils-dev
		fi
	elif [ "${OPERATIVE_SYSTEM}" = "Darwin" ]; then
		brew install dwarfutils
		brew install libelf
		mkdir -p build
		CMAKE_CONFIG_PATH="$ROOT_DIR/build/CMakeConfig.txt"
		LIBDWARD_PREFIX=$(brew --prefix dwarfutils)
		LIBELF_PREFIX=$(brew --prefix libelf)
		echo "-DLIBDWARF_INCLUDE_DIR=${LIBDWARD_PREFIX}/include" >> $CMAKE_CONFIG_PATH
		echo "-DLIBELF_LIBRARY=${LIBELF_PREFIX}/lib/libelf.a" >> $CMAKE_CONFIG_PATH
		echo "-DLIBELF_INCLUDE_DIR=${LIBELF_PREFIX}/include" >> $CMAKE_CONFIG_PATH
	fi
}

# Sandbox (this provides sandboxing features in Linux through BFS filters with libseccomp)
sub_sandbox(){
	echo "configure sandbox"
	cd $ROOT_DIR

	if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
		if [ "${LINUX_DISTRO}" = "debian" ] || [ "${LINUX_DISTRO}" = "ubuntu" ]; then
			$SUDO_CMD apt-get install -y --no-install-recommends libseccomp-dev
		elif [ "${LINUX_DISTRO}" = "alpine" ]; then
			$SUDO_CMD apk add --no-cache libseccomp-dev
		fi
	fi
}

# Install
sub_install(){
	if [ $APT_CACHE = 1 ]; then
		if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
			APT_CACHE_CMD=-o dir::cache::archives="$APT_CACHE_DIR"
		fi
	fi
	if [ $INSTALL_BASE = 1 ]; then
		sub_base
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
	if [ $INSTALL_NETCORE5 = 1 ]; then
		sub_netcore5
	fi
	if [ $INSTALL_NETCORE7 = 1 ]; then
		sub_netcore7
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
	if [ $INSTALL_GO = 1 ]; then
		sub_go
	fi
	if [ $INSTALL_RUST = 1 ]; then
		sub_rust
	fi
	if [ $INSTALL_SWIG = 1 ]; then
		sub_swig
	fi
	if [ $INSTALL_PACK = 1 ]; then
		sub_pack
	fi
	if [ $INSTALL_COVERAGE = 1 ]; then
		sub_coverage
	fi
	if [ $INSTALL_CLANGFORMAT = 1 ]; then
		sub_clangformat
	fi
	if [ $INSTALL_BACKTRACE = 1 ]; then
		sub_backtrace
	fi
	if [ $INSTALL_SANDBOX = 1 ]; then
		sub_sandbox
	fi
	echo "install finished in workspace $ROOT_DIR"
}

# Configuration
sub_options(){
	for option in "$@"
	do
		if [ "$option" = 'cache' ]; then
			if [ "${OPERATIVE_SYSTEM}" = "Linux" ]; then
				echo "apt caching selected"
				APT_CACHE=1
			fi
		fi
		if [ "$option" = 'debug' ]; then
			echo "Install dependencies in debug mode"
			BUILD_TYPE=Debug
		fi
		if [ "$option" = 'release' ] || [ "$option" = 'relwithdebinfo' ]; then
			echo "Install dependencies release mode"
			BUILD_TYPE=Release
		fi
		if [ "$option" = 'base' ]; then
			echo "apt selected"
			INSTALL_BASE=1
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
		if [ "$option" = 'netcore7' ]; then
			echo "netcore 7 selected"
			INSTALL_NETCORE7=1
		fi
		if [ "$option" = 'rapidjson' ]; then
			echo "rapidjson selected"
			INSTALL_RAPIDJSON=1
		fi
		if [ "$option" = 'funchook' ]; then
			echo "funchook selected"
			INSTALL_FUNCHOOK=1
		fi
		if [ "$option" = 'v8' ] || [ "$option" = 'v8rep54' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO54=1
		fi
		if [ "$option" = 'v8rep57' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO57=1
		fi
		if [ "$option" = 'v8rep58' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO58=1
		fi
		if [ "$option" = 'v8rep52' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO52=1
		fi
		if [ "$option" = 'v8rep51' ]; then
			echo "v8 selected"
			INSTALL_V8REPO=1
			INSTALL_V8REPO51=1
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
		if [ "$option" = 'go' ]; then
			echo "go selected"
			INSTALL_GO=1
		fi
		if [ "$option" = 'rust' ]; then
			echo "rust selected"
			INSTALL_RUST=1
		fi
		if [ "$option" = 'swig' ]; then
			echo "swig selected"
			INSTALL_SWIG=1
		fi
		if [ "$option" = 'pack' ]; then
			echo "pack selected"
			INSTALL_PACK=1
		fi
		if [ "$option" = 'coverage' ]; then
			echo "coverage selected"
			INSTALL_COVERAGE=1
		fi
		if [ "$option" = 'clangformat' ]; then
			echo "clangformat selected"
			INSTALL_CLANGFORMAT=1
		fi
		if [ "$option" = 'backtrace' ]; then
			echo "backtrace selected"
			INSTALL_BACKTRACE=1
		fi
		if [ "$option" = 'sandbox' ]; then
			echo "sandbox selected"
			INSTALL_SANDBOX=1
		fi
	done
}

# Help
sub_help() {
	echo "Usage: `basename "$0"` list of component"
	echo "Components:"
	echo "	debug | release | relwithdebinfo"
	echo "	cache"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	netcore2"
	echo "	netcore5"
	echo "	netcore7"
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
	echo "	rpc"
	echo "	wasm"
	echo "	java"
	echo "	c"
	echo "	cobol"
	echo "	go"
	echo "	swig"
	echo "	pack"
	echo "	coverage"
	echo "	clangformat"
	echo "	backtrace"
	echo "	sandbox"
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
