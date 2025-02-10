<#
#	MetaCall Build PowerShell Script by Parra Studios
#	Build and install powershell script utility for MetaCall.
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
#>

$PSDefaultParameterValues['*:Encoding'] = 'utf8'

$Global:ROOT_DIR = "$(Get-Location)"
$Global:SHOW_HELP = 0
$Global:PROGNAME = $(Get-Item $PSCommandPath).Basename
$Global:Arguments = $args

function Set-Python {
	Write-Output "Install Python"
	Set-Location $ROOT_DIR

	$PythonVersion = '3.9.7'
	$RuntimeDir = "$env:ProgramFiles\Python3"
	$DepsDir = "$ROOT_DIR\dependencies"

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
	./python_installer.exe /quiet "TargetDir=$RuntimeDir" Include_debug=1 Include_symbols=1 PrependPath=1 CompileAll=1

	# Set environment variables
	Add-to-Path $RuntimeDir
	Add-to-Path "$RuntimeDir\Scripts"
	[Environment]::SetEnvironmentVariable("PIP_TARGET", "$RuntimeDir\Lib")

	# No patch, save vars for later use
	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$PythonRuntimeDir = $RuntimeDir.Replace('\', '/')

	Write-Output "-DPython3_VERSION=$PythonVersion" >> $EnvOpts
	Write-Output "-DPython3_ROOT_DIR=""$PythonRuntimeDir""" >> $EnvOpts
	Write-Output "-DPython3_EXECUTABLE=""$PythonRuntimeDir/python.exe""" >> $EnvOpts
	Write-Output "-DPython3_INCLUDE_DIRS=""$PythonRuntimeDir/include""" >> $EnvOpts
	Write-Output "-DPython3_LIBRARIES=""$PythonRuntimeDir/libs/python39_d.lib;$PythonRuntimeDir/libs/python39.lib""" >> $EnvOpts
	Write-Output "-DPython3_Development_FOUND=1" >> $EnvOpts
	Write-Output "-DPython3_FIND_REGISTRY=NEVER" >> $EnvOpts

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
	Write-Output "Install Node.js"
	Set-Location $ROOT_DIR

	$DepsDir = "$ROOT_DIR\dependencies"
	$NodeVersion = "14.18.2"
	$DLLReleaseVer = "v0.0.1"
	$RuntimeDir = "$env:ProgramFiles\nodejs"

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

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	Write-Output "-DNodeJS_VERSION=""$NodeVersion""" >> $EnvOpts
	Write-Output "-DNodeJS_INCLUDE_DIRS=""$NodeDir/include/node""" >> $EnvOpts
	Write-Output "-DNodeJS_LIBRARY=""$NodeDir/lib/libnode.lib""" >> $EnvOpts
	Write-Output "-DNodeJS_EXECUTABLE=""$NodeDir/node.exe""" >> $EnvOpts
	Write-Output "-DNodeJS_LIBRARY_NAME=""libnode.dll""" >> $EnvOpts
	Write-Output "-DNodeJS_LIBRARY_NAME_PATH=""$NodeDir/lib/libnode.dll""" >> $EnvOpts

	if ($Arguments -contains "c") {
		# Required for test source/tests/metacall_node_port_c_lib_test
		if (!(Test-Path -Path "$DepsDir\libgit2")) {
			# Clone libgit2
			git clone --depth 1 --branch v1.8.4 https://github.com/libgit2/libgit2
		}

		$InstallDir = "$DepsDir\libgit2\build\dist"

		mkdir "$DepsDir\libgit2\build"
		mkdir "$InstallDir"
		Set-Location "$DepsDir\libgit2\build"

		cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF -DBUILD_CLI=OFF ..
		cmake --build . "-j$((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)"
		cmake --install . --prefix "$InstallDir"

		Write-Output "-DLibGit2_LIBRARY=""$InstallDir\lib\git2.lib""" >> $EnvOpts
		Write-Output "-DLibGit2_INCLUDE_DIR=""$InstallDir\include""" >> $EnvOpts
	}
}

function Set-Java {
	Write-Output "Install Java"
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

function Set-Ruby {
	Write-Output "Install Ruby"
	$RUBY_VERSION = "3.1.2"	

	Set-Location $ROOT_DIR
	$RuntimeDir = "$env:ProgramFiles\ruby"
	$DepsDir = "$ROOT_DIR\dependencies"

	if (!(Test-Path -Path "$DepsDir\ruby-mswin.7z")) {
		# Download installer
		Write-Output "Ruby not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://github.com/MSP-Greg/ruby-mswin/releases/download/ruby-mswin-builds/Ruby-$RUBY_VERSION-ms.7z", "$DepsDir\ruby-mswin.7z")
	}

	mkdir "$DepsDir\Ruby31-ms"
	7z x "$DepsDir\ruby-mswin.7z" -o"$DepsDir"

	robocopy /move /e "$DepsDir\Ruby31-ms\" $RuntimeDir

	Add-to-Path "$RuntimeDir\bin"

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$RubyDir  = $RuntimeDir.Replace('\', '/')

	Write-Output "-DRuby_VERSION_STRING=""$RUBY_VERSION""" >> $EnvOpts
	Write-Output "-DRuby_INCLUDE_DIR=""$RubyDir/include/ruby-3.1.0""" >> $EnvOpts
	Write-Output "-DRuby_EXECUTABLE=""$RubyDir/bin/ruby.exe""" >> $EnvOpts
	Write-Output "-DRuby_LIBRARY=""$RubyDir/lib/x64-vcruntime140-ruby310.lib""" >> $EnvOpts
	Write-Output "-DRuby_LIBRARY_NAME=""$RubyDir/bin/x64-vcruntime140-ruby310.dll""" >> $EnvOpts
}

function Install-VS {
    Write-Output "Checking for preinstalled Visual Studio Build Tools..."
    
    # Path to vswhere.exe
    $vsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    # Use vswhere to search for a Visual Studio instance with the VC++ tools
    if (Test-Path $vsWherePath) {
        $vsInstance = & $vsWherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    }
    
    if (-not $vsInstance) {
        Write-Output "Visual Studio Build Tools with VC++ components not found. Installing..."
        choco install visualstudio2019buildtools -y
        Write-Output "Installation initiated. Please reboot if required and re-run the script."
    } else {
        Write-Output "Found Visual Studio at: $vsInstance"
    }
}

function Set-C {
    $ErrorActionPreference = "Stop"
    Write-Output "Installing C dependencies..."

    # Ensure Chocolatey is set up
    Set-Choco

    # Install Visual Studio Build Tools
    Install-VS

    # Set directories
    Set-Location $ROOT_DIR
    $DepsDir = Join-Path $ROOT_DIR 'dependencies'
    $vcpkgDir = Join-Path $DepsDir 'vcpkg'

	# Install vcpkg if missing
    if (-not (Test-Path (Join-Path $vcpkgDir 'vcpkg.exe'))) {
        Write-Output "Cloning vcpkg into $vcpkgDir..."
        git clone https://github.com/microsoft/vcpkg.git $vcpkgDir
        & (Join-Path $vcpkgDir 'bootstrap-vcpkg.bat')
        & (Join-Path $vcpkgDir 'vcpkg.exe') integrate install
    } else {
        Write-Output "vcpkg already installed."
    }

    # Install libffi using vcpkg
    Write-Output "Installing libffi using vcpkg..."
    & (Join-Path $vcpkgDir 'vcpkg.exe') install libffi

    # Install LLVM using Chocolatey
    Write-Output "Installing LLVM..."
    choco install llvm -y

    # Clone and build libtcc
    $tccRepoUrl = "https://github.com/TinyCC/tinycc.git"
    $tccDestination = "$DepsDir\tcc"
    if (-not (Test-Path "$tccDestination\.git")) {
        Write-Output "Cloning libtcc..."
        git clone $tccRepoUrl $tccDestination
    } else {
        Write-Output "libtcc already cloned."
    }

    # Build libtcc using Git Bash
    $gitBashPath = "C:\Program Files\Git\bin\bash.exe"
    if ($gitBashPath) {
        Write-Output "Building libtcc..."
        Set-Location $tccDestination
		& "$gitBashPath" -c "git config --global core.autocrlf input"
		$bashCommand = "cd /c/gcc-14.2.0/bin/ && gcc --version"
        & "$gitBashPath" -c "$bashCommand" | Tee-Object -Variable output
		& "$gitBashPath" -c "sed -i 's/\r$//' configure"
        & "$gitBashPath" -c "ls && ./configure && make && make test && make install" | Tee-Object -Variable output
    } else {
        Write-Output "Git Bash not found. Please install Git for Windows."
    }

    # Write environment options for CMake configuration
    $Env_Opts = "$ROOT_DIR\build\CMakeConfig.txt"
    $LLVM_Dir = "$env:ProgramFiles\LLVM"
    $vcpkgLibDir = "$DepsDir\vcpkg\installed\x64-windows\lib"
    $vcpkgIncludeDir = "$DepsDir\vcpkg\installed\x64-windows\include"
    $tccLib = "$tccDestination\lib\tcc.lib"
    $tccInclude = "$tccDestination\include"

    $cmakeOptions = @(
        "-DLIBFFI_LIBRARY=$vcpkgLibDir\libffi.lib"
        "-DLIBFFI_INCLUDE_DIR=$vcpkgIncludeDir"
        "-DLibClang_INCLUDE_DIR=$LLVM_Dir\include\clang"
        "-DLIBCLANG_LIBRARY=$LLVM_Dir\lib\libclang.lib"
        "-DTCC_LIBRARY=$tccLib"
        "-DTCC_INCLUDE_DIR=$tccInclude"
    )

    $cmakeOptions | Out-File -Append -FilePath $Env_Opts

    Write-Output "All dependencies installed and configured successfully."
}



function Clone-GitRepository {
    param (
        [string]$repositoryUrl,
        [string]$destinationPath
    )

    # Check if Git is installed
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        Write-Error "Git is not installed. Please install Git and try again."
        return
    }

    # Check if the destination path already exists
    if (Test-Path $destinationPath) {
        Write-Error "Destination path already exists. Please provide a different path."
        return
    }

    # Clone the repository using Git
    & git clone $repositoryUrl $destinationPath

    # Check if the cloning was successful
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to clone the repository."
        return
    }

    Write-Output "Repository cloned successfully."
}

function Set-TypeScript {
	Write-Output "Install TypeScript"
	npm i react@latest -g
	npm i react-dom@latest -g
}

function Set-Curl {
	Write-Output "Installing cURL"

	Set-Location $ROOT_DIR
	$RuntimeDir = "$env:ProgramFiles\curl"
	$DepsDir = "$ROOT_DIR\dependencies"
	$Version = "8.3.0_2"

	if (!(Test-Path -Path "$DepsDir\curl.zip")) {
		# Download installer
		Write-Output "Curl not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://curl.se/windows/dl-$Version/curl-$Version-win64-mingw.zip", "$DepsDir\curl.zip")
	}

	Set-Location $DepsDir

	7z x "$DepsDir\curl.zip"

	robocopy /move /e "$DepsDir\curl-$Version-win64-mingw" $RuntimeDir

	Add-to-Path "$RuntimeDir\bin"

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$CurlDir  = $RuntimeDir.Replace('\', '/')

	$CURL_INCLUDE_DIR="$CurlDir/include"
	$CURL_LIB="$CurlDir/lib/libcurl.dll.a"
	$CURL_LIB_NAME="$CurlDir/bin/libcurl-x64.dll"

	Write-Output "-DCURL_INCLUDE_DIR=""$CURL_INCLUDE_DIR""" >> $EnvOpts
	Write-Output "-DCURL_INCLUDE_DIRS=""$CURL_INCLUDE_DIR""" >> $EnvOpts
	Write-Output "-DCURL_LIBRARY=""$CURL_LIB""" >> $EnvOpts
	Write-Output "-DCURL_LIBRARY_NAME=""$CURL_LIB_NAME""" >> $EnvOpts
}

function Add-to-Path {
	$GivenPath = $args[0]

	$NewPath = "$GivenPath;$Env:PATH"
	setx /M PATH $NewPath
	$Env:PATH = $NewPath
	$GivenPath >> $env:GITHUB_PATH

	if ($Null -ne $Env:GITHUB_ENV) {
		Write-Output "PATH=$Env:PATH" >> $Env:GITHUB_ENV
	}

	refreshenv

	Write-Output "PATH:: " $Env:PATH
}

function Set-Base {
	$DepsDir = "$ROOT_DIR\dependencies"

	# Check if 7zip is installed
	if (!(Get-ItemProperty HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\* | ?{$_.DisplayName -like "7-Zip*"})) {
		Write-Output "Install 7zip"

		if (!(Test-Path -Path "$DepsDir\7zip.exe")) {
			# Download installer
			(New-Object Net.WebClient).DownloadFile("https://www.7-zip.org/a/7z2201-x64.exe", "$DepsDir\7zip.exe")
		}

		# https://gist.github.com/dansmith65/7dd950f183af5f5deaf9650f2ad3226c
		$installerPath = "$DepsDir\7zip.exe"
		Start-Process -FilePath $installerPath -Args "/S" -Verb RunAs -Wait
		Add-to-Path "$env:ProgramFiles\7-Zip"
	}
}

function Set-Choco {
	# Set directory for installation - Chocolatey does not lock
	# down the directory if not the default
	$InstallDir='C:\ProgramData\chocoportable'
	$env:ChocolateyInstall="$InstallDir"

	# If your PowerShell Execution policy is restrictive, you may
	# not be able to get around that. Try setting your session to
	# Bypass.
	Set-ExecutionPolicy Bypass -Scope Process -Force;

	# All install options - offline, proxy, etc at
	# https://chocolatey.org/install
	iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
}
# Configure
function Configure {
	# Create option variables file 
	mkdir "$ROOT_DIR\build"
	New-Item -Path "$ROOT_DIR\build\CMakeConfig.txt"

	# Install base requirements
	Set-Base

	for ($i = 0; $i -lt $Arguments.Length; $i++) {
		$var = $Arguments[$i]
		if ("$var" -eq 'python') {
			Write-Output "python selected"
			Set-Python
		}
		if ("$var" -eq 'ruby') {
			Write-Output "ruby selected"
			Set-Ruby
		}
		if ("$var" -eq 'netcore') {
			Write-Output "netcore selected"
		}
		if ("$var" -eq 'netcore2') {
			Write-Output "netcore 2 selected"
		}
		if ("$var" -eq 'netcore5') {
			Write-Output "netcore 5 selected"
		}
		if ("$var" -eq 'rapidjson') {
			Write-Output "rapidjson selected"
		}
		if ("$var" -eq 'funchook') {
			Write-Output "funchook selected"
		}
		if (("$var" -eq 'v8') -or ("$var" -eq 'v8rep54')) {
			Write-Output "v8 selected"
		}
		if ("$var" -eq 'v8rep57') {
			Write-Output "v8 selected"
		}
		if ("$var" -eq 'v8rep58') {
			Write-Output "v8 selected"
		}
		if ("$var" -eq 'v8rep52') {
			Write-Output "v8 selected"
		}
		if ("$var" -eq 'v8rep51') {
			Write-Output "v8 selected"
		}
		if ("$var" -eq 'nodejs') {
			Write-Output "nodejs selected"
			Set-Nodejs
		}
		if ("$var" -eq 'typescript') {
			Write-Output "typescript selected"
			Set-TypeScript
		}
		if ("$var" -eq 'file') {
			Write-Output "file selected"
		}
		if ("$var" -eq 'rpc') {
			Write-Output "rpc selected"
			Set-Curl
		}
		if ("$var" -eq 'wasm') {
			Write-Output "wasm selected"
		}
		if ("$var" -eq 'java') {
			Write-Output "java selected"
			Set-Java
		}
		if ("$var" -eq 'c') {
			Write-Output "c selected"
			Set-C
		}
		if ("$var" -eq 'cobol') {
			Write-Output "cobol selected"
		}
		if ("$var" -eq 'go') {
			Write-Output "go selected"
		}
		if ("$var" -eq 'rust') {
			Write-Output "rust selected"
		}
		if ("$var" -eq 'swig') {
			Write-Output "swig selected"
		}
		if ("$var" -eq 'metacall') {
			Write-Output "metacall selected"
		}
		if ("$var" -eq 'pack') {
			Write-Output "pack selected"
		}
		if ("$var" -eq 'clangformat') {
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
