$PSDefaultParameterValues['*:Encoding'] = 'utf8'

$Global:ROOT_DIR = "$(Get-Location)"
$Global:SHOW_HELP = 0
$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename
$Global:Arguments = $args

function Set-Python {
    Write-Output "configure python"
	Set-Location $ROOT_DIR

	$PythonVersion = '3.9.7'
	$RuntimeDir    = "$env:ProgramFiles\python"
	$DepsDir       = "$ROOT_DIR\dependencies"

	mkdir -Force $DepsDir
	mkdir -Force $RuntimeDir
	Set-Location $DepsDir

    if (!(Test-Path -Path "$DepsDir\python_installer.exe")) {
		# Download installer
		Write-Output "Python installer not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://www.python.org/ftp/python/$PythonVersion/python-$PythonVersion-amd64.exe", "$(Get-Location)\python_installer.exe")
    }

	Write-Output "Installing python $PythonVersion"

	# Install Python
	./python_installer.exe /quiet "TargetDir=$RuntimeDir" `
		Include_debug=1 Include_symbols=1 PrependPath=1 CompileAll=1

	# Set environment variables
	Add-to-Path $RuntimeDir
	Add-to-Path "$RuntimeDir\Scripts"
	[Environment]::SetEnvironmentVariable("PIP_TARGET", "$RuntimeDir\Lib")

	# No patch, save vars for later use
	$Env_Opts = "$ROOT_DIR\build\env_vars.txt"
	$PythonRuntimeDir = $RuntimeDir.Replace('\', '/')

	Write-Output "-DPython_VERSION=$PythonVersion" >> $Env_Opts
	Write-Output "-DPython_ROOT_DIR=""$PythonRuntimeDir""" >> $Env_Opts
	Write-Output "-DPython_EXECUTABLE=""$PythonRuntimeDir/python.exe""" >> $Env_Opts
	Write-Output "-DPython_INCLUDE_DIRS=""$PythonRuntimeDir/include""" >> $Env_Opts
	Write-Output "-DPython_LIBRARIES=""$PythonRuntimeDir/libs/python39_d.lib;$PythonRuntimeDir/libs/python39.lib""" >> $Env_Opts

	$FindPython = "$ROOT_DIR\cmake\FindPython.cmake"

	Write-Output "FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python REQUIRED_VARS Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS VERSION_VAR Python_VERSION)" >> $FindPython
	Write-Output "mark_as_advanced(Python_EXECUTABLE Python_LIBRARIES Python_INCLUDE_DIRS)" >> $FindPython

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

function Set-Nodejs {
	Write-Output "Setting up Node.js"
	Set-Location $ROOT_DIR

	$DepsDir = "$ROOT_DIR\dependencies"
	$NodeVersion = "14.18.2"
	$DLLReleaseVer = "v0.0.1"
	$RuntimeDir    = "$env:ProgramFiles\nodejs"

	Set-Location $DepsDir

	if (!(Test-Path -Path "$DepsDir\node.msi")) {
		# Download installer
		Write-Output "Nodejs MSI installer not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://nodejs.org/download/release/v$NodeVersion/node-v$NodeVersion-x64.msi", "$DepsDir\node.msi")
    }

	if (!(Test-Path -Path "$DepsDir\node_headers.tar.gz")) {
		# Download installer
		Write-Output "Nodejs headers installer not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://nodejs.org/download/release/v$NodeVersion/node-v$NodeVersion-headers.tar.gz", "$DepsDir\node_headers.tar.gz")
    }

	msiexec.exe /quiet /i "$DepsDir\node.msi" 

	Add-to-Path $RuntimeDir
	Add-to-Path "$RuntimeDir\include"

	cmake -E tar xzf node_headers.tar.gz

	mkdir "$RuntimeDir\include"
	Robocopy.exe /move /e "$DepsDir\node-v$NodeVersion\include" "$RuntimeDir\include" /NFL /NDL /NJH /NJS /NC /NS /NP
	Set-Location $DepsDir

	if (!(Test-Path -Path "$DepsDir\node_dll.zip")) {
		# Download installer
		Write-Output "Nodejs Custom DLLs not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://github.com/metacall/node.dll/releases/download/$DLLReleaseVer/node-shared-v$NodeVersion-x64.zip", "$DepsDir\node_dll.zip")
    }

	Expand-Archive -Path "node_dll.zip" -DestinationPath "$RuntimeDir\lib"
	
	$NodeDir  = $RuntimeDir.Replace('\', '/')

	$Env_Opts = "$ROOT_DIR\build\env_vars.txt"
	Write-Output "-DNodeJS_VERSION=""$NodeVersion""" >> $Env_Opts
	Write-Output "-DNodeJS_INCLUDE_DIRS=""$NodeDir/include/node""" >> $Env_Opts
	Write-Output "-DNodeJS_LIBRARY=""$NodeDir/lib/libnode.lib""" >> $Env_Opts
	Write-Output "-DNodeJS_EXECUTABLE=""$NodeDir/node.exe""" >> $Env_Opts
	Write-Output "-DNodeJS_LIBRARY_NAME=""libnode.dll""" >> $Env_Opts
}

function Set-Java {
		Write-Output "Setting up Java..."
		$JAVA_VERSION = "17.0.5"
		$RuntimeDir = "$env:ProgramFiles\openjdk"
		$DepsDir = "$ROOT_DIR\dependencies"

		Set-Location $DepsDir

		if (!(Test-Path -Path "$DepsDir\openjdk.zip")) {
			# Download installer
			Write-Output "OpenJDK not found downloading now..."
			(New-Object Net.WebClient).DownloadFile("https://aka.ms/download-jdk/microsoft-jdk-$JAVA_VERSION-windows-x64.zip", "$DepsDir\openjdk.zip")
		}

		Expand-Archive -Path "openjdk.zip" -DestinationPath "$RuntimeDir"
		robocopy /move /e "$RuntimeDir\jdk-$JAVA_VERSION+8" "$RuntimeDir" /NFL /NDL /NJH /NJS /NC /NS /NP

		Add-to-Path "JAVA_HOME=$RuntimeDir"
		Add-to-Path "$RuntimeDir\bin"
		Add-to-Path "$RuntimeDir\bin\server"
}

function Add-to-Path {
	$GivenPath = $args[0]

	$NewPath = "$GivenPath;$Env:PATH"
	setx /M PATH $NewPath
	$Env:PATH = $NewPath

	$GivenPath >> $env:GITHUB_PATH

	if ( $Null -ne $Env:GITHUB_ENV ) {
		Write-Output "PATH=$Env:PATH" >> $Env:GITHUB_ENV
		# echo "{$Env:PATH}" >> $Env:GITHUB_PATH # Doesn't work
	}

	refreshenv

	Write-Output "PATH:: " $Env:PATH
}

# Configure
function Configure {
	# Create option variables file 
	mkdir "$ROOT_DIR\build"
	New-Item -Path "$ROOT_DIR\build\env_vars.txt"
	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$var = $Arguments[$i]
		if ( "$var" -eq 'python' ) {
			Write-Output "python selected"
			Set-Python
		}
		if ( "$var" -eq 'ruby' ) {
			Write-Output "ruby selected"
		}
		if ( "$var" -eq 'netcore' ) {
			Write-Output "netcore selected"
		}
		if ( "$var" -eq 'netcore2' ) {
			Write-Output "netcore 2 selected"
		}
		if ( "$var" -eq 'netcore5' ) {
			Write-Output "netcore 5 selected"
		}
		if ( "$var" -eq 'rapidjson' ) {
			Write-Output "rapidjson selected"
		}
		if ( "$var" -eq 'funchook' ) {
			Write-Output "funchook selected"
		}
		if ( ("$var" -eq 'v8') -or ("$var" -eq 'v8rep54') ) {
			Write-Output "v8 selected"
		}
		if ( "$var" -eq 'v8rep57' ) {
			Write-Output "v8 selected"
		}
		if ( "$var" -eq 'v8rep58' ) {
			Write-Output "v8 selected"
		}
		if ( "$var" -eq 'v8rep52' ) {
			Write-Output "v8 selected"
		}
		if ( "$var" -eq 'v8rep51' ) {
			Write-Output "v8 selected"
		}
		if ( "$var" -eq 'nodejs' ) {
			Write-Output "nodejs selected"
			Set-Nodejs
		}
		if ( "$var" -eq 'typescript' ) {
			Write-Output "typescript selected"
		}
		if ( "$var" -eq 'file' ) {
			Write-Output "file selected"
		}
		if ( "$var" -eq 'rpc' ) {
			Write-Output "rpc selected"
		}
		if ( "$var" -eq 'wasm' ) {
			Write-Output "wasm selected"
		}
		if ( "$var" -eq 'java' ) {
			Write-Output "java selected"
			Set-Java
		}
		if ( "$var" -eq 'c' ) {
			Write-Output "c selected"
		}
		if ( "$var" -eq 'cobol' ) {
			Write-Output "cobol selected"
		}
		if ( "$var" -eq 'go' ) {
			Write-Output "go selected"
		}
		if ( "$var" -eq 'rust' ) {
			Write-Output "rust selected"
		}
		if ( "$var" -eq 'swig' ) {
			Write-Output "swig selected"
		}
		if ( "$var" -eq 'metacall' ) {
			Write-Output "metacall selected"
		}
		if ( "$var" -eq 'pack' ) {
			Write-Output "pack selected"
		}
		if ( "$var" -eq 'coverage' ) {
			Write-Output "coverage selected"
		}
		if ( "$var" -eq 'clangformat' ) {
			Write-Output "clangformat selected"
		}
	}
}

# Help
function Help {
	Write-Output "Usage: $PROGNAME list of component"
	Write-Output "Components:"
	Write-Output "	python"
	Write-Output "	ruby"
	Write-Output "	netcore"
	Write-Output "	netcore2"
	Write-Output "	netcore5"
	Write-Output "	rapidjson"
	Write-Output "	funchook"
	Write-Output "	v8"
	Write-Output "	v8rep51"
	Write-Output "	v8rep54"
	Write-Output "	v8rep57"
	Write-Output "	v8rep58"
	Write-Output "	nodejs"
	Write-Output "	typescript"
	Write-Output "	file"
	Write-Output "	rpc"
	Write-Output "	wasm"
	Write-Output "	java"
	Write-Output "	c"
	Write-Output "	cobol"
	Write-Output "	go"
	Write-Output "	swig"
	Write-Output "	metacall"
	Write-Output "	pack"
	Write-Output "	coverage"
	Write-Output "	clangformat"
	Write-Output ""
}

switch($args.length) {
	0 {
		Help
		Break
	}
	Default {
		Configure
	}
}
