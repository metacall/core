<#
#	MetaCall Build PowerShell Script by Parra Studios
#	Build and install powershell script utility for MetaCall.
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

	if (!(Test-Path -Path "$DepsDir\curl.zip")) {
		# Download installer
		Write-Output "Curl not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://curl.se/windows/dl-8.1.2_3/curl-8.1.2_3-win64-mingw.zip", "$DepsDir\curl.zip")
	}

	Set-Location $DepsDir

	7z x "$DepsDir\curl.zip"

	robocopy /move /e "$DepsDir\curl-8.1.2_3-win64-mingw" $RuntimeDir

	Add-to-Path "$RuntimeDir\bin"

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$CurlDir  = $RuntimeDir.Replace('\', '/')

	$CURL_INCLUDE_DIR="$CurlDir/include"
	$CURL_LIB="$CurlDir/lib/libcurl.dll.a"
	$CURL_LIB_NAME="$CurlDir/bin/libcurl-x64.dll"

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
		if ("$var" -eq 'coverage') {
			Write-Output "coverage selected"
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
