$PSDefaultParameterValues['*:Encoding'] = 'utf8'
$Global:ProgressPreference = 'SilentlyContinue'

$Global:ROOT_DIR = "$(pwd)"

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

# Helper functions
function Add-to-Path {
	$GivenPath = $args[0]

	$NewPath = "$GivenPath;$Env:PATH"
	setx /M PATH $NewPath
	$Env:PATH = $NewPath

	if ( $Env:GITHUB_ENV -ne $Null ) {
		echo "PATH=$Env:PATH" >> $Env:GITHUB_ENV
		# echo "{$Env:PATH}" >> $Env:GITHUB_PATH # Doesn't work
	}

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

	$PythonVersion = '3.9.7'
	$RuntimeDir    = "$ROOT_DIR\build\runtimes\python"
	$DepsDir       = "$ROOT_DIR\dependencies"

	md -Force $DepsDir
	md -Force $RuntimeDir
	cd $DepsDir

	# Download installer
	(New-Object Net.WebClient).DownloadFile("https://www.python.org/ftp/python/$PythonVersion/python-$PythonVersion-amd64.exe", "$(pwd)\python_installer.exe")

	# Install Python
	./python_installer.exe /quiet "TargetDir=$RuntimeDir" `
		Include_debug=1 Include_symbols=1 PrependPath=1 CompileAll=1

	# Set environment variables
	Add-to-Path $RuntimeDir
	[Environment]::SetEnvironmentVariable("PIP_TARGET", "$RuntimeDir\Lib")

	# Patch for FindPython.cmake
	$FindPython = "$ROOT_DIR\cmake\FindPython.cmake"
	$PythonRuntimeDir = $RuntimeDir.Replace('\', '/')

	echo "set(Python_VERSION $PythonVersion)"                             > $FindPython
	echo "set(Python_ROOT_DIR ""$PythonRuntimeDir"")"                    >> $FindPython
	echo "set(Python_EXECUTABLE ""$PythonRuntimeDir/python.exe"")"       >> $FindPython
	echo "set(Python_INCLUDE_DIRS ""$PythonRuntimeDir/include"")"        >> $FindPython
	echo "set(Python_LIBRARIES ""$PythonRuntimeDir/libs/python39_d.lib"" ""$PythonRuntimeDir/libs/python39.lib"")" >> $FindPython
	echo "include(FindPackageHandleStandardArgs)"                        >> $FindPython
	echo "FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python REQUIRED_VARS Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS VERSION_VAR Python_VERSION)" >> $FindPython
	echo "mark_as_advanced(Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS)" >> $FindPython

	# Install dependencies for tests
	pip3 install requests
	pip3 install setuptools
	pip3 install wheel
	pip3 install rsa
	pip3 install scipy
	pip3 install numpy
	pip3 install scikit-learn
	pip3 install joblib
}

# Ruby
function sub-ruby {
	echo "configure ruby"
	cd $ROOT_DIR

	$DepsDir               = "$ROOT_DIR\dependencies"
	$RubyDownloadVersion   = '3.1.2'
	$RubyVersion           = '3.1.0'
	$RuntimeDir            = "$ROOT_DIR\build\runtimes\ruby"

	md -Force $DepsDir
	md -Force $RuntimeDir
	cd $DepsDir

	# Download
	(New-Object Net.WebClient).DownloadFile("https://github.com/MSP-Greg/ruby-mswin/releases/download/ruby-mswin-builds/Ruby-$RubyDownloadVersion-ms.7z", "$(pwd)\ruby-mswin.7z")

	# Install Ruby
	7z x "$DepsDir\ruby-mswin.7z"
	robocopy /move /e "$DepsDir\Ruby31-ms" $RuntimeDir /NFL /NDL /NJH /NJS /NC /NS /NP

	Add-to-Path "$RuntimeDir\bin"
	
	# Patch for FindRuby.cmake
	$FindRuby = "$ROOT_DIR\cmake\FindRuby.cmake"
	$RubyRuntimeDir = $RuntimeDir.Replace('\', '/')

	echo "set(Ruby_VERSION $RubyVersion)"                                          > $FindRuby
	echo "set(Ruby_ROOT_DIR ""$RubyRuntimeDir"")"                                 >> $FindRuby
	echo "set(Ruby_EXECUTABLE ""$RubyRuntimeDir/bin/ruby.exe"")"                  >> $FindRuby
	echo "set(Ruby_INCLUDE_DIRS ""$RubyRuntimeDir/include/ruby-$RubyVersion;$RubyRuntimeDir/include/ruby-$RubyVersion/x64-mswin64_140"")" >> $FindRuby
	echo "set(Ruby_LIBRARY ""$RubyRuntimeDir/lib/x64-vcruntime140-ruby310.lib"")" >> $FindRuby
	echo "include(FindPackageHandleStandardArgs)"                                 >> $FindRuby
	echo "FIND_PACKAGE_HANDLE_STANDARD_ARGS(Ruby REQUIRED_VARS Ruby_EXECUTABLE Ruby_LIBRARY Ruby_INCLUDE_DIRS VERSION_VAR Ruby_VERSION)"  >> $FindRuby
	echo "mark_as_advanced(Ruby_EXECUTABLE Ruby_LIBRARY Ruby_INCLUDE_DIRS)"       >> $FindRuby

	# TODO: This should be done by CMake
	# # Move DLL to correct location (to be done AFTER build)
	# # mv -Force "$RuntimeDir\bin\x64-vcruntime140-ruby310.dll" "$ROOT_DIR\lib"
	# md -Force "$ROOT_DIR\lib"
	# cp -Force "$RuntimeDir\bin\x64-vcruntime140-ruby310.dll" "$ROOT_DIR\lib\"
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

	$DotNetDownloadVersion = '5.0.403'
	$DotNetVersion         = '5.0.12'
	$RuntimeDir            = "$ROOT_DIR\build\runtimes\dotnet"
	$DepsDir               = "$ROOT_DIR\dependencies"

	md -Force $DepsDir
	md -Force $RuntimeDir
	cd $DepsDir

	# Download SDK
	(New-Object Net.WebClient).DownloadFile("https://download.visualstudio.microsoft.com/download/pr/d1ca6dbf-d054-46ba-86d1-36eb2e455ba2/e950d4503116142d9c2129ed65084a15/dotnet-sdk-$DotNetDownloadVersion-win-x64.zip", "$(pwd)\dotnet_sdk.zip")

	# Install .NET
	Expand-Archive -Path "dotnet_sdk.zip" -DestinationPath $RuntimeDir
	git clone --branch v5.0.12 --depth 1 --single-branch https://github.com/dotnet/runtime.git "$RuntimeDir\runtime"
	md -Force "$RuntimeDir\include"
	robocopy /move /e "$RuntimeDir\runtime\src\coreclr\src\pal" "$RuntimeDir\include\pal" /NFL /NDL /NJH /NJS /NC /NS /NP
	robocopy /move /e "$RuntimeDir\runtime\src\coreclr\src\inc" "$RuntimeDir\include\inc" /NFL /NDL /NJH /NJS /NC /NS /NP
	rd -Recurse -Force "$RuntimeDir\runtime"

	Add-to-Path $RuntimeDir

	# Patch for FindDotNET.cmake
	$FindDotNet = "$ROOT_DIR\cmake\FindDotNET.cmake"
	$DotNetDir  = $RuntimeDir.Replace('\', '/')

	echo "set(DOTNET_VERSION $DotNetVersion)"                             >  $FindDotNet
	echo "set(DOTNET_MIGRATE 1)"                                          >> $FindDotNet
	echo "set(DOTNET_COMMAND ""$DotNetDir/dotnet.exe"")"                  >> $FindDotNet
	echo "include(FindPackageHandleStandardArgs)"                         >> $FindDotNet
	echo "FIND_PACKAGE_HANDLE_STANDARD_ARGS(DotNET REQUIRED_VARS DOTNET_COMMAND DOTNET_MIGRATE VERSION_VAR DOTNET_VERSION)" >> $FindDotNet
	echo "mark_as_advanced(DOTNET_COMMAND DOTNET_MIGRATE DOTNET_VERSION)" >> $FindDotNet
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
	echo "configure nodejs"
	cd $ROOT_DIR

	# Configuration
	$NASMVer    = '2.15.05'
	
	$DepsDir       = "$ROOT_DIR\dependencies"
	$NodeVersion   = '14.18.2'
	$DLLReleaseVer = 'v0.0.1'
	$RuntimeDir    = "$ROOT_DIR\build\runtimes\nodejs"

	# Install NASM
	(New-Object Net.WebClient).DownloadFile("https://www.nasm.us/pub/nasm/releasebuilds/$NASMVer/win64/nasm-$NASMVer-win64.zip", "$(pwd)\nasm.zip")
	Expand-Archive -Path 'nasm.zip' -DestinationPath $RuntimeDir

	$NASMDir = "$RuntimeDir\nasm-$NASMVer"

	Add-to-Path "$NASMDir\rdoff"
	Add-to-Path $NASMDir

	# Install NodeJS

	md -Force $DepsDir
	md -Force $RuntimeDir
	cd $DepsDir

	# Download
	(New-Object Net.WebClient).DownloadFile("https://nodejs.org/download/release/v$NodeVersion/node-v$NodeVersion-win-x64.zip", "$(pwd)\node.zip")
	(New-Object Net.WebClient).DownloadFile("https://nodejs.org/download/release/v$NodeVersion/node-v$NodeVersion-headers.tar.gz", "$(pwd)\node_headers.tar.gz")

	# Install runtime
	Expand-Archive -Path "node.zip" -DestinationPath $RuntimeDir
	robocopy /move /e "$RuntimeDir\node-v$NodeVersion-win-x64" "$RuntimeDir" /NFL /NDL /NJH /NJS /NC /NS /NP
	rd -Recurse -Force "$RuntimeDir\node-v$NodeVersion-win-x64"

	Add-to-Path $RuntimeDir

	# Install headers
	cmake -E tar xzf node_headers.tar.gz
	cd "$DepsDir\node-v$NodeVersion"
	md "$RuntimeDir\include"
	robocopy /move /e "$DepsDir\node-v$NodeVersion\include" "$RuntimeDir\include" /NFL /NDL /NJH /NJS /NC /NS /NP
	cd $DepsDir
	rd -Recurse -Force "$DepsDir\node-v$NodeVersion"

	# Install custom Node DLL
	(New-Object Net.WebClient).DownloadFile("https://github.com/metacall/node.dll/releases/download/$DLLReleaseVer/node-shared-v$NodeVersion-x64.zip", "$(pwd)\node_dll.zip")
	Expand-Archive -Path "node_dll.zip" -DestinationPath "$RuntimeDir\lib"

	# Patch for FindNodeJS.cmake
	$FindNode = "$ROOT_DIR\cmake\FindNodeJS.cmake"
	$NodeDir  = $RuntimeDir.Replace('\', '/')

	echo "set(NodeJS_VERSION $NodeVersion)"                   >> $FindNode
	echo "set(NodeJS_INCLUDE_DIRS ""$NodeDir/include/node"")" >> $FindNode
	echo "set(NodeJS_LIBRARY ""$NodeDir/lib/libnode.lib"")"   >> $FindNode
	echo "set(NodeJS_EXECUTABLE ""$NodeDir/node.exe"")"       >> $FindNode
	echo "set(NodeJS_LIBRARY_NAME ""libnode.dll"")"           >> $FindNode
	echo "include(FindPackageHandleStandardArgs)"             >> $FindNode
	echo "FIND_PACKAGE_HANDLE_STANDARD_ARGS(NodeJS REQUIRED_VARS NodeJS_INCLUDE_DIRS NodeJS_LIBRARY NodeJS_EXECUTABLE VERSION_VAR NodeJS_VERSION)" >> $FindNode
	echo "mark_as_advanced(NodeJS_VERSION NodeJS_INCLUDE_DIRS NodeJS_LIBRARY NodeJS_EXECUTABLE)" >> $FindNode
}

# TypeScript
function sub-typescript {
	echo "configure typescript"
	npm i react@latest -g
	npm i react-dom@latest -g
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
	$JAVA_VERSION = "17.0.5"
	$RuntimeDir = "$ROOT_DIR\build\runtimes\openjdk"

	(New-Object Net.WebClient).DownloadFile("https://aka.ms/download-jdk/microsoft-jdk-$JAVA_VERSION-windows-x64.zip", "$(pwd)\openjdk.zip");
	Expand-Archive -Path "openjdk.zip" -DestinationPath "$RuntimeDir"
	robocopy /move /e "$RuntimeDir\jdk-$JAVA_VERSION+8" "$RuntimeDir" /NFL /NDL /NJH /NJS /NC /NS /NP

	# Setting JAVA_HOME as a system level variable adding to doesn't work
	# [Environment]::SetEnvironmentVariable('JAVA_HOME', $RuntimeDir, 'Machine') -> this need elevated permissions

	"JAVA_HOME=$RuntimeDir" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
	"$RuntimeDir\bin" >> $env:GITHUB_PATH
	
	# Add jvm.dll to path else loader fails while loading, also copying this dll won't work
	# since jvm.dll requires other dlls from the openjdk and then it fails to load its deps
	# Add-to-Path "$RuntimeDir\bin\server"
	"$RuntimeDir\bin\server" >> $env:GITHUB_PATH

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
