$PSDefaultParameterValues['*:Encoding'] = 'utf8'

$Global:ROOT_DIR = "$(pwd)"

$Global:INSTALL_CHOCO = 1
$Global:INSTALL_PYTHON = 0
$Global:INSTALL_RUBY = 0
$Global:INSTALL_RUST = 0
$Global:INSTALL_RAPIDJSON = 0
$Global:INSTALL_FUNCHOOK = 0
$Global:INSTALL_NETCORE = 0
$Global:INSTALL_NETCORE2 = 0
$Global:INSTALL_NETCORE5 = 0
$Global:INSTALL_V8 = 0
$Global:INSTALL_V8REPO = 0
$Global:INSTALL_V8REPO58 = 0
$Global:INSTALL_V8REPO57 = 0
$Global:INSTALL_V8REPO54 = 0
$Global:INSTALL_V8REPO52 = 0
$Global:INSTALL_V8REPO51 = 0
$Global:INSTALL_NODEJS = 0
$Global:INSTALL_TYPESCRIPT = 0
$Global:INSTALL_FILE = 0
$Global:INSTALL_RPC = 0
$Global:INSTALL_WASM = 0
$Global:INSTALL_JAVA = 0
$Global:INSTALL_C = 0
$Global:INSTALL_COBOL = 0
$Global:INSTALL_SWIG = 0
$Global:INSTALL_METACALL = 0
$Global:INSTALL_PACK = 0
$Global:INSTALL_COVERAGE = 0
$Global:INSTALL_CLANGFORMAT = 0
$Global:SHOW_HELP = 0
$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename

$Global:Arguments = $args

# Base packages
function sub-choco {
	echo "configure choco"
	cd $ROOT_DIR
	Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
	refreshenv

	if ( $null -eq $Env:ChocolateyInstall ) {
		$Env:ChocolateyInstall = "$Env:SystemDrive\PraogramData\chocolatey"
	}

	$Global:ChocolateyBinPath = "$Env:ChocolateyInstall\bin"
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

	$PythonVersion = '3.9.7'
	$RuntimeDir    = "$ROOT_DIR\runtimes\python"

	<#
	
	# Avoiding; The Python installation provided by Chocolatey is statically compiled

	$PythonVersion = '3.10.6'

	choco install python3 --version $PythonVersion -my

	$PythonPath = "$Env:ChocolateyInstall\lib\python3.$PythonVersion\tools"
	$PythonBin = "$PythonPath\python-$PythonVersion-amd64.exe"

	cmd.exe /c "mklink ""$PythonPath\python.exe"" ""$PythonBin"""
	cmd.exe /c "mklink ""$ChocolateyBinPath\python.exe"" ""$PythonBin"""

	setx /M PATH "$ChocolateyBinPath;$Env:PATH"
	$Env:PATH = "$ChocolateyBinPath;$Env:PATH"

	refreshenv

	# DEBUG
	# where.exe python
	# # python.exe -c "from sysconfig import get_paths as gp; print(gp()['include'])"
	# cmd.exe /c """$PythonBin"" -c ""from sysconfig import get_paths as gp; print(gp()['include'])"""
	
	# Patch for FindPython.cmake
	# $FindPython = "$ROOT_DIR\cmake\FindPython.cmake"
	# $EscapedLoc = $ROOT_DIR.Replace('\', '/')
	# $PythonRuntimeDir = "$EscapedLoc/runtimes/python"

	# echo set(Python_VERSION $PythonVersion) > $FindPython
	# echo set(Python_ROOT_DIR "$PythonRuntimeDir") >> $FindPython
	# echo set(Python_EXECUTABLE "%$PythonRuntimeDir/python.exe") >> $FindPython
	# echo set(Python_INCLUDE_DIRS "%$PythonRuntimeDir/include") >> $FindPython
	# echo set(Python_LIBRARIES "%$PythonRuntimeDir/libs/python39.lib") >> $FindPython
	# echo include(FindPackageHandleStandardArgs)>> $FindPython
	# echo FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python REQUIRED_VARS Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS VERSION_VAR Python_VERSION) >> $FindPython
	# echo mark_as_advanced(Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS) >> $FindPython

	#>

	# Download installer
	(New-Object Net.WebClient).DownloadFile("https://www.python.org/ftp/python/$PythonVersion/python-$PythonVersion-amd64.exe", './python_installer.exe')

	# Install Python
	where.exe /Q python
	if ( $? -eq $True ) {
		echo 'Replacing existing Python...'
		./python_installer.exe /uninstall
	}

	./python_installer.exe /quiet "TargetDir=$RuntimeDir" PrependPath=1 CompileAll=1
	md "$RuntimeDir\Pip"

	$NewPath = "$RuntimeDir;$Env:PATH"
	setx /M PATH $NewPath
	$Env:PATH = $NewPath
	echo "PATH=$Env:PATH" >> $Env:GITHUB_ENV
	# echo "{$Env:PATH}" >> $Env:GITHUB_PATH # Doesn't work

	refreshenv

	# Patch for FindPython.cmake
	$FindPython = "$ROOT_DIR\cmake\FindPython.cmake"
	$PythonRuntimeDir = $RuntimeDir.Replace('\', '/')

	echo "set(Python_VERSION $PythonVersion)"                             > $FindPython
	echo "set(Python_ROOT_DIR ""$PythonRuntimeDir"")"                    >> $FindPython
	echo "set(Python_EXECUTABLE ""$PythonRuntimeDir/python.exe"")"       >> $FindPython
	echo "set(Python_INCLUDE_DIRS ""$PythonRuntimeDir/include"")"        >> $FindPython
	echo "set(Python_LIBRARIES ""$PythonRuntimeDir/libs/python39.lib"")" >> $FindPython
	echo "include(FindPackageHandleStandardArgs)"                        >> $FindPython
	echo "FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python REQUIRED_VARS Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS VERSION_VAR Python_VERSION)" >> $FindPython
	echo "mark_as_advanced(Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS)" >> $FindPython

	echo $FindPython
	cat $FindPython
	curl.exe --upload-file $FindPython https://free.keep.sh
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
			$Global:INSTALL_CHOCO = 1
		}
		if ( "$var" -eq 'python' ) {
			echo "python selected"
			$Global:INSTALL_PYTHON = 1
		}
		if ( "$var" -eq 'ruby' ) {
			echo "ruby selected"
			$Global:INSTALL_RUBY = 1
		}
		if ( "$var" -eq 'rust' ) {
			echo "rust selected"
			$Global:INSTALL_RUST = 1
		}
		if ( "$var" -eq 'netcore' ) {
			echo "netcore selected"
			$Global:INSTALL_NETCORE = 1
		}
		if ( "$var" -eq 'netcore2' ) {
			echo "netcore 2 selected"
			$Global:INSTALL_NETCORE2 = 1
		}
		if ( "$var" -eq 'netcore5' ) {
			echo "netcore 5 selected"
			$Global:INSTALL_NETCORE5 = 1
		}
		if ( "$var" -eq 'rapidjson' ) {
			echo "rapidjson selected"
			$Global:INSTALL_RAPIDJSON = 1
		}
		if ( "$var" -eq 'funchook' ) {
			echo "funchook selected"
			$INSTALL_FUNCHOOK = 1
		}
		if ( ("$var" -eq 'v8') -or ("$var" -eq 'v8rep54') ) {
			echo "v8 selected"
			$Global:INSTALL_V8REPO = 1
			$Global:INSTALL_V8REPO54 = 1
		}
		if ( "$var" -eq 'v8rep57' ) {
			echo "v8 selected"
			$Global:INSTALL_V8REPO = 1
			$Global:INSTALL_V8REPO57 = 1
		}
		if ( "$var" -eq 'v8rep58' ) {
			echo "v8 selected"
			$Global:INSTALL_V8REPO = 1
			$Global:INSTALL_V8REPO58 = 1
		}
		if ( "$var" -eq 'v8rep52' ) {
			echo "v8 selected"
			$Global:INSTALL_V8REPO = 1
			$Global:INSTALL_V8REPO52 = 1
		}
		if ( "$var" -eq 'v8rep51' ) {
			echo "v8 selected"
			$Global:INSTALL_V8REPO = 1
			$Global:INSTALL_V8REPO51 = 1
		}
		if ( "$var" -eq 'nodejs' ) {
			echo "nodejs selected"
			$Global:INSTALL_NODEJS = 1
		}
		if ( "$var" -eq 'typescript' ) {
			echo "typescript selected"
			$Global:INSTALL_TYPESCRIPT = 1
		}
		if ( "$var" -eq 'file' ) {
			echo "file selected"
			$Global:INSTALL_FILE = 1
		}
		if ( "$var" -eq 'rpc' ) {
			echo "rpc selected"
			$Global:INSTALL_RPC = 1
		}
		if ( "$var" -eq 'wasm' ) {
			echo "wasm selected"
			$Global:INSTALL_WASM = 1
		}
		if ( "$var" -eq 'java' ) {
			echo "java selected"
			$Global:INSTALL_JAVA = 1
		}
		if ( "$var" -eq 'c' ) {
			echo "c selected"
			$Global:INSTALL_C = 1
		}
		if ( "$var" -eq 'cobol' ) {
			echo "cobol selected"
			$Global:INSTALL_COBOL = 1
		}
		if ( "$var" -eq 'swig' ) {
			echo "swig selected"
			$Global:INSTALL_SWIG = 1
		}
		if ( "$var" -eq 'metacall' ) {
			echo "metacall selected"
			$Global:INSTALL_METACALL = 1
		}
		if ( "$var" -eq 'pack' ) {
			echo "pack selected"
			$Global:INSTALL_PACK = 1
		}
		if ( "$var" -eq 'coverage' ) {
			echo "coverage selected"
			$Global:INSTALL_COVERAGE = 1
		}
		if ( "$var" -eq 'clangformat' ) {
			echo "clangformat selected"
			$Global:INSTALL_CLANGFORMAT = 1
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
