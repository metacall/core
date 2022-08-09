$global:ROOT_DIR = "$(pwd)"

$global:INSTALL_CHOCO = 1
$global:INSTALL_PYTHON = 0
$global:INSTALL_RUBY = 0
$global:INSTALL_RUST = 0
$global:INSTALL_RAPIDJSON = 0
$global:INSTALL_FUNCHOOK = 0
$global:INSTALL_NETCORE = 0
$global:INSTALL_NETCORE2 = 0
$global:INSTALL_NETCORE5 = 0
$global:INSTALL_V8 = 0
$global:INSTALL_V8REPO = 0
$global:INSTALL_V8REPO58 = 0
$global:INSTALL_V8REPO57 = 0
$global:INSTALL_V8REPO54 = 0
$global:INSTALL_V8REPO52 = 0
$global:INSTALL_V8REPO51 = 0
$global:INSTALL_NODEJS = 0
$global:INSTALL_TYPESCRIPT = 0
$global:INSTALL_FILE = 0
$global:INSTALL_RPC = 0
$global:INSTALL_WASM = 0
$global:INSTALL_JAVA = 0
$global:INSTALL_C = 0
$global:INSTALL_COBOL = 0
$global:INSTALL_SWIG = 0
$global:INSTALL_METACALL = 0
$global:INSTALL_PACK = 0
$global:INSTALL_COVERAGE = 0
$global:INSTALL_CLANGFORMAT = 0
$global:SHOW_HELP = 0
$global:PROGNAME = $(Get-Item $PSCommandPath).Basename

$global:Arguments = $args

# Base packages
function sub-choco {
	echo "configure choco"
	cd $ROOT_DIR
	Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
	refreshenv
}

# Swig
function sub-swig {
	echo "configure swig"
	cd $ROOT_DIR
	
}

# Python
function sub-python {
	echo "configure python"
	cd $ROOT_DIR
	
}

# Ruby
function sub-ruby {
	echo "configure ruby"
	cd $ROOT_DIR
	
	# TODO (copied from metacall-environment.sh): Review conflict with NodeJS (currently rails test is disabled)

}

# Rust
function sub-rust {
	echo "configure rust"
	cd $ROOT_DIR
	
}

# RapidJSON
function sub-rapidjson {
	echo "configure rapidjson"
	cd $ROOT_DIR
	
}

# FuncHook
function sub-funchook {
	echo "configure funchook"
	
}

# NetCore
function sub-netcore {
	echo "configure netcore"
	cd $ROOT_DIR
	
}

# NetCore 2
function sub-netcore2 {
	echo "configure netcore 2"
	cd $ROOT_DIR
	
}

# NetCore 5
function sub-netcore5 {
	echo "configure netcore 5"
	cd $ROOT_DIR
	
}

# V8 Repository
function sub-v8repo {
	echo "configure v8 from repository"
	cd $ROOT_DIR
	
}

# V8
function sub-v8 {
	echo "configure v8"
	cd $ROOT_DIR
	
}

# NodeJS
function sub-nodejs {
	# TODO (copied from metacall-environment.sh): Review conflicts with Ruby Rails and NodeJS 4.x
	echo "configure nodejs"
	cd $ROOT_DIR
	
}

# TypeScript
function sub-typescript {
	echo "configure typescript"
	
}

# File
function sub-file {
	echo "configure file"
}

# RPC
function sub-rpc {
	echo "cofingure rpc"
	cd $ROOT_DIR
	
}

# WebAssembly
function sub-wasm {
	echo "configure webassembly"
	# TODO (copied from metacall-environment.sh)
}

# Java
function sub-java {
	echo "configure java"
	
}

# C
function sub-c {
	echo "configure c"
	
}

# Cobol
function sub-cobol {
	echo "configure cobol"
	
}

# MetaCall
function sub-metacall {
	# TODO (copied from metacall-environment.sh): Update this or deprecate it
	echo "configure metacall"
	cd $ROOT_DIR
	
}

# Pack
function sub-pack {
	echo "configure pack"
	cd $ROOT_DIR
	
}

# Coverage
function sub-coverage {
	echo "configure coverage"
	cd $ROOT_DIR
	
}

# Clang format
function sub-clangformat {
	echo "configure clangformat"
	cd $ROOT_DIR

}

# Install
function sub-install {
	if ( $INSTALL_CHOCO -eq 1 ) {
		sub-choco
	}
	if ( $INSTALL_PYTHON -eq 1 ) {
		sub-python
	}
	if ( $INSTALL_RUBY -eq 1 ) {
		sub-ruby
	}
	if ( $INSTALL_RUST -eq 1 ) {
		sub-rust
	}
	if ( $INSTALL_RAPIDJSON -eq 1 ) {
		sub-rapidjson
	}
	if ( $INSTALL_FUNCHOOK -eq 1 ) {
		sub-funchook
	}
	if ( $INSTALL_NETCORE -eq 1 ) {
		sub-netcore
	}
	if ( $INSTALL_NETCORE2 -eq 1 ) {
		sub-netcore2
	}
	if ( $INSTALL_NETCORE5 -eq 1 ) {
		sub-netcore5
	}
	if ( $INSTALL_V8 -eq 1 ) {
		sub-v8
	}
	if ( $INSTALL_V8REPO -eq 1 ) {
		sub-v8repo
	}
	if ( $INSTALL_NODEJS -eq 1 ) {
		sub-nodejs
	}
	if ( $INSTALL_TYPESCRIPT -eq 1 ) {
		sub-typescript
	}
	if ( $INSTALL_FILE -eq 1 ) {
		sub-file
	}
	if ( $INSTALL_RPC -eq 1 ) {
		sub-rpc
	}
	if ( $INSTALL_WASM -eq 1 ) {
		sub-wasm
	}
	if ( $INSTALL_JAVA -eq 1 ) {
		sub-java
	}
	if ( $INSTALL_C -eq 1 ) {
		sub-c
	}
	if ( $INSTALL_COBOL -eq 1 ) {
		sub-cobol
	}
	if ( $INSTALL_SWIG -eq 1 ) {
		sub-swig
	}
	if ( $INSTALL_METACALL -eq 1 ) {
		sub-metacall
	}
	if ( $INSTALL_PACK -eq 1 ) {
		sub-pack
	}
	if ( $INSTALL_COVERAGE -eq 1 ) {
		sub-coverage
	}
	if ( $INSTALL_CLANGFORMAT -eq 1 ) {
		sub-clangformat
	}
	echo "install finished in workspace $ROOT_DIR"
}

# Configuration
function sub-options {
	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$var = $Arguments[$i]
		if ( "$var" -eq 'base' ) {
			echo "choco selected"
			$INSTALL_CHOCO = 1
		}
		if ( "$var" -eq 'python' ) {
			echo "python selected"
			$INSTALL_PYTHON = 1
		}
		if ( "$var" -eq 'ruby' ) {
			echo "ruby selected"
			$INSTALL_RUBY = 1
		}
		if ( "$var" -eq 'rust' ) {
			echo "rust selected"
			$INSTALL_RUST = 1
		}
		if ( "$var" -eq 'netcore' ) {
			echo "netcore selected"
			$INSTALL_NETCORE = 1
		}
		if ( "$var" -eq 'netcore2' ) {
			echo "netcore 2 selected"
			$INSTALL_NETCORE2 = 1
		}
		if ( "$var" -eq 'netcore5' ) {
			echo "netcore 5 selected"
			$INSTALL_NETCORE5 = 1
		}
		if ( "$var" -eq 'rapidjson' ) {
			echo "rapidjson selected"
			$INSTALL_RAPIDJSON = 1
		}
		if ( "$var" -eq 'funchook' ) {
			echo "funchook selected"
			$INSTALL_FUNCHOOK = 1
		}
		if ( ("$var" -eq 'v8') -or ("$var" -eq 'v8rep54') ) {
			echo "v8 selected"
			$INSTALL_V8REPO = 1
			$INSTALL_V8REPO54 = 1
		}
		if ( "$var" -eq 'v8rep57' ) {
			echo "v8 selected"
			$INSTALL_V8REPO = 1
			$INSTALL_V8REPO57 = 1
		}
		if ( "$var" -eq 'v8rep58' ) {
			echo "v8 selected"
			$INSTALL_V8REPO = 1
			$INSTALL_V8REPO58 = 1
		}
		if ( "$var" -eq 'v8rep52' ) {
			echo "v8 selected"
			$INSTALL_V8REPO = 1
			$INSTALL_V8REPO52 = 1
		}
		if ( "$var" -eq 'v8rep51' ) {
			echo "v8 selected"
			$INSTALL_V8REPO = 1
			$INSTALL_V8REPO51 = 1
		}
		if ( "$var" -eq 'nodejs' ) {
			echo "nodejs selected"
			$INSTALL_NODEJS = 1
		}
		if ( "$var" -eq 'typescript' ) {
			echo "typescript selected"
			$INSTALL_TYPESCRIPT = 1
		}
		if ( "$var" -eq 'file' ) {
			echo "file selected"
			$INSTALL_FILE = 1
		}
		if ( "$var" -eq 'rpc' ) {
			echo "rpc selected"
			$INSTALL_RPC = 1
		}
		if ( "$var" -eq 'wasm' ) {
			echo "wasm selected"
			$INSTALL_WASM = 1
		}
		if ( "$var" -eq 'java' ) {
			echo "java selected"
			$INSTALL_JAVA = 1
		}
		if ( "$var" -eq 'c' ) {
			echo "c selected"
			$INSTALL_C = 1
		}
		if ( "$var" -eq 'cobol' ) {
			echo "cobol selected"
			$INSTALL_COBOL = 1
		}
		if ( "$var" -eq 'swig' ) {
			echo "swig selected"
			$INSTALL_SWIG = 1
		}
		if ( "$var" -eq 'metacall' ) {
			echo "metacall selected"
			$INSTALL_METACALL = 1
		}
		if ( "$var" -eq 'pack' ) {
			echo "pack selected"
			$INSTALL_PACK = 1
		}
		if ( "$var" -eq 'coverage' ) {
			echo "coverage selected"
			$INSTALL_COVERAGE = 1
		}
		if ( "$var" -eq 'clangformat' ) {
			echo "clangformat selected"
			$INSTALL_CLANGFORMAT = 1
		}
	}
}

# Help
function sub-help {
	echo "Usage: $PROGNAME list of component"
	echo "Components:"
	echo "	base"
	echo "	python"
	echo "	ruby"
	echo "	netcore"
	echo "	netcore2"
	echo "	netcore5"
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
	echo "	swig"
	echo "	metacall"
	echo "	pack"
	echo "	coverage"
	echo "	clangformat"
	echo ""
}

switch($args.length) {
	0 {
		sub-help
		Break
	}
	Default {
		sub-options
		sub-install
	}
}
