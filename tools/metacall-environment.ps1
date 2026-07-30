<#
#	MetaCall Build PowerShell Script by Parra Studios
#	Build and install powershell script utility for MetaCall.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

	$PythonVersion = '3.10.2'
	$Version = [version]$PythonVersion
	$MajorMinor = "$($Version.Major)$($Version.Minor)"
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
	Write-Output "-DPython3_LIBRARIES=""$PythonRuntimeDir/libs/python${MajorMinor}_d.lib;$PythonRuntimeDir/libs/python${MajorMinor}.lib""" >> $EnvOpts
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
	$NodeVersion = "20.11.0"
	$DLLReleaseVer = "v0.0.6"
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
			if ($LASTEXITCODE -ne 0) {
				throw "Failed to clone libgit2 (exit code $LASTEXITCODE)."
			}
		}

		$InstallDir = "$DepsDir\libgit2\build\dist"
		$LibGit2Config = "Debug"

		mkdir "$DepsDir\libgit2\build"
		mkdir "$InstallDir"
		Set-Location "$DepsDir\libgit2\build"

		cmake "-DCMAKE_BUILD_TYPE=$LibGit2Config" -DBUILD_TESTS=OFF -DBUILD_CLI=OFF ..
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to configure libgit2 (exit code $LASTEXITCODE)."
		}

		cmake --build . --config $LibGit2Config --parallel $((Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors)
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to build libgit2 (exit code $LASTEXITCODE)."
		}

		cmake --install . --config $LibGit2Config --prefix "$InstallDir"
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to install libgit2 (exit code $LASTEXITCODE)."
		}

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

	$Env:JAVA_HOME = $RuntimeDir
	if ($Null -ne $Env:GITHUB_ENV) {
		Write-Output "JAVA_HOME=$RuntimeDir" >> $Env:GITHUB_ENV
	}

	Add-to-Path "$RuntimeDir\bin"
	Add-to-Path "$RuntimeDir\bin\server"
}

function Set-Ruby {
	Write-Output "Install Ruby"

	Set-Location $ROOT_DIR
	$RuntimeDir = "$env:ProgramFiles\ruby"
	$DepsDir = "$ROOT_DIR\dependencies"

	if (!(Test-Path -Path "$DepsDir\ruby-mswin.7z")) {
		# Download installer
		Write-Output "Ruby not found downloading now..."
		(New-Object Net.WebClient).DownloadFile("https://github.com/metacall/ruby-loco/releases/download/ruby-master/ruby-mswin.7z", "$DepsDir\ruby-mswin.7z")
	}

	mkdir "$DepsDir\ruby-mswin"
	7z x "$DepsDir\ruby-mswin.7z" -o"$DepsDir"

	robocopy /move /e "$DepsDir\ruby-mswin\" $RuntimeDir

	Add-to-Path "$RuntimeDir\bin"

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$RubyDir  = $RuntimeDir.Replace('\', '/')

	Write-Output "-DRuby_VERSION_STRING=""3.5.0""" >> $EnvOpts
	Write-Output "-DRuby_INCLUDE_DIR=""$RubyDir/include/ruby-3.5.0+0""" >> $EnvOpts
	Write-Output "-DRuby_EXECUTABLE=""$RubyDir/bin/ruby.exe""" >> $EnvOpts
	Write-Output "-DRuby_LIBRARY=""$RubyDir/lib/x64-vcruntime140-ruby350.lib""" >> $EnvOpts
	Write-Output "-DRuby_LIBRARY_NAME=""$RubyDir/bin/x64-vcruntime140-ruby350.dll""" >> $EnvOpts
	Write-Output "-DRuby_LIBRARY_SEARCH_PATHS=""$RubyDir/bin/ruby_builtin_dlls""" >> $EnvOpts
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
	$Version = "8.19.0_7"

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

function Set-C {
	Write-Output "Install C Loader Dependencies (libffi + libclang)"

	Set-Location $ROOT_DIR

	$LibFFIVersion = "3.5.2"
	$LibFFISha256 = "f3a3082a23b37c293a4fcd1053147b371f2ff91fa7ea1b2a52e335676bac82dc"
	$LLVMVersion = "19.1.0"
	$DepsDir = "$ROOT_DIR\dependencies"
	$RuntimeDir = "$DepsDir\libffi"
	$Archive = "$DepsDir\libffi.tar.gz"
	$SrcDir = "$DepsDir\libffi-$LibFFIVersion"
	$BuildDir = "$RuntimeDir\build"
	$DistDir = "$RuntimeDir\dist"

	mkdir -Force $DepsDir
	mkdir -Force $RuntimeDir

	Set-Location $DepsDir

	# Download official libffi source tarball
	if (!(Test-Path -Path $Archive)) {
		Write-Output "libffi not found, downloading..."
		(New-Object Net.WebClient).DownloadFile(
			"https://github.com/libffi/libffi/releases/download/v$LibFFIVersion/libffi-$LibFFIVersion.tar.gz",
			$Archive
		)
	}

	$LibFFIHashAlgorithm = [System.Security.Cryptography.SHA256]::Create()
	try {
		$LibFFIArchiveStream = [System.IO.File]::OpenRead($Archive)
		try {
			$LibFFIHashBytes = $LibFFIHashAlgorithm.ComputeHash($LibFFIArchiveStream)
			$LibFFIArchiveHash = ([System.BitConverter]::ToString($LibFFIHashBytes)).Replace("-", "").ToLowerInvariant()
		}
		finally {
			$LibFFIArchiveStream.Dispose()
		}
	}
	finally {
		$LibFFIHashAlgorithm.Dispose()
	}

	if ($LibFFIArchiveHash -ne $LibFFISha256) {
		throw "libffi archive checksum mismatch: expected $LibFFISha256, got $LibFFIArchiveHash."
	}

	# Always configure from clean generated directories. This prevents a
	# previous shared or failed build from contaminating the static build.
	foreach ($GeneratedDir in @($SrcDir, $BuildDir, $DistDir)) {
		if (Test-Path -Path $GeneratedDir) {
			Remove-Item -Path $GeneratedDir -Recurse -Force
		}
	}

	cmake -E tar xzf $Archive
	if ($LASTEXITCODE -ne 0) {
		throw "Failed to extract libffi (exit code $LASTEXITCODE)."
	}

	mkdir -Force $BuildDir
	mkdir -Force $DistDir

	$GitBash = "C:\Program Files\Git\bin\bash.exe"

	if (!(Test-Path -Path $GitBash)) {
		throw "Git Bash is required to build libffi but was not found at '$GitBash'."
	}

	$LibFFIRequiredFiles = @(
		"configure",
		"msvcc.sh",
		"ltmain.sh",
		"compile",
		"missing",
		"install-sh",
		"config.guess",
		"config.sub",
		"include\ffi.h.in"
	)

	foreach ($RequiredFile in $LibFFIRequiredFiles) {
		if (!(Test-Path -Path "$SrcDir\$RequiredFile")) {
			throw "The libffi source archive is missing '$RequiredFile'."
		}
	}

	foreach ($RequiredCommand in @("cl.exe", "link.exe", "lib.exe", "ml64.exe", "make.exe")) {
		if ($null -eq (Get-Command $RequiredCommand -ErrorAction SilentlyContinue)) {
			throw "The required libffi build tool '$RequiredCommand' is not available in PATH."
		}
	}

	$env:LIBFFI_SRC = $SrcDir
	$env:LIBFFI_BUILD = $BuildDir
	$env:LIBFFI_INSTALL = $DistDir

	$LibFFIBuildScript = @'
		set -euxo pipefail

		src="$(cygpath -u "$LIBFFI_SRC")"
		build="$(cygpath -u "$LIBFFI_BUILD")"
		install="$(cygpath -u "$LIBFFI_INSTALL")"
		src_native="$(cygpath -m "$LIBFFI_SRC")"

		# The release archive already contains generated Autotools files. Avoid
		# timestamp rounding on Windows triggering their regeneration.
		find "$src" -name Makefile.in -exec touch {} +
		touch "$src/fficonfig.h.in"
		sleep 3

		cd "$build"

		SHELL=/bin/sh CONFIG_SHELL=/bin/sh "$src/configure" \
			CC="$src/msvcc.sh -m64" \
			CXX="$src/msvcc.sh -m64" \
			LD=link \
			CPP="cl -nologo -EP" \
			CXXCPP="cl -nologo -EP" \
			CPPFLAGS="-DFFI_STATIC_BUILD" \
			--disable-docs \
			--disable-shared \
			--enable-static \
			--prefix="$install"

		test -f "$src/include/ffi.h.in"
		grep '^SHELL = /bin/sh$' Makefile

		# GNU Make on Windows expands $(SHELL) to Git's installation path, which
		# contains spaces. The source archive's generated rules must also remain
		# inert so Make cannot launch an Autotools regeneration/recheck loop.
		find "$src" "$build" \
			\( -name Makefile.in -o -name Makefile \) -exec \
			sed -i \
				-e 's|$(SHELL)|/bin/sh|g' \
				-e 's|^AUTOMAKE =.*|AUTOMAKE = true|' \
				-e 's|^ACLOCAL =.*|ACLOCAL = true|' \
				-e 's|^AUTOCONF =.*|AUTOCONF = true|' \
				-e 's|^AUTOHEADER =.*|AUTOHEADER = true|' \
				-e 's|^\($(srcdir)/Makefile\.in\):.*|\1:|' \
				-e 's|^\($(top_srcdir)/configure\):.*|\1:|' \
				-e 's|^\($(ACLOCAL_M4)\):.*|\1:|' \
				-e 's|^\($(top_srcdir)/fficonfig\.h\.in\):.*|\1:|' \
				-e 's|^\([^:#]*config\.status\):.*|\1:|' \
				-e 's|^\(Makefile\):.*|\1:|' \
				-e 's|^\([^:#]*stamp-h1\):.*|\1:|' \
				{} +

		# Configure requires MSYS paths, while the native MinGW Make executable
		# performs dependency checks using Windows paths.
		find "$build" -name Makefile -exec \
			sed -i "s|$src|$src_native|g" {} +

		grep -F '$(srcdir)/Makefile.in:' Makefile
		grep -E 'config\.status:$' Makefile
		grep '^Makefile:$' Makefile
		grep -E 'stamp-h1:$' Makefile

		make V=1 \
			SHELL=/bin/sh \
			CONFIG_SHELL=/bin/sh \
			AUTOMAKE=true \
			ACLOCAL=true \
			AUTOCONF=true \
			AUTOHEADER=true

		make V=1 install \
			SHELL=/bin/sh \
			CONFIG_SHELL=/bin/sh \
			AUTOMAKE=true \
			ACLOCAL=true \
			AUTOCONF=true \
			AUTOHEADER=true

		find "$install" -maxdepth 4 -type f -print
'@

	& $GitBash -lc $LibFFIBuildScript

	if ($LASTEXITCODE -ne 0) {
		throw "Failed to configure, build, or install libffi (exit code $LASTEXITCODE)."
	}

	$LibFFIHeader = Get-ChildItem -Path $DistDir -Filter "ffi.h" -File -Recurse | Select-Object -First 1
	$LibFFITargetHeader = Get-ChildItem -Path $DistDir -Filter "ffitarget.h" -File -Recurse | Select-Object -First 1
	$LibFFILibrary = Get-ChildItem -Path $DistDir -Filter "*.lib" -File -Recurse | Where-Object { $_.Name -match '^libffi' } | Select-Object -First 1

	if (($null -eq $LibFFIHeader) -or ($null -eq $LibFFITargetHeader) -or ($null -eq $LibFFILibrary)) {
		throw "libffi installation is incomplete: expected ffi.h, ffitarget.h, and an MSVC libffi .lib file under '$DistDir'."
	}

	$EnvOpts = "$ROOT_DIR\build\CMakeConfig.txt"
	$LibFFIIncludeDir = $LibFFIHeader.Directory.FullName.Replace('\', '/')
	$LibFFILibraryPath = $LibFFILibrary.FullName.Replace('\', '/')

	Write-Output "-DLIBFFI_INCLUDE_DIR=""$LibFFIIncludeDir""" >> $EnvOpts
	Write-Output "-DLIBFFI_LIBRARY=""$LibFFILibraryPath""" >> $EnvOpts
	Write-Output "-DLIBFFI_STATIC_BUILD=On" >> $EnvOpts

	# Download official LLVM prebuilt archive (includes libclang.lib + headers)
	# Using clang+llvm archive which contains libraries needed for development
	$LLVMArchive = "clang+llvm-$LLVMVersion-x86_64-pc-windows-msvc.tar.xz"
	$LLVMDir = "$DepsDir\llvm"

	if (!(Test-Path -Path "$DepsDir\$LLVMArchive")) {
		Write-Output "LLVM not found, downloading..."
		(New-Object Net.WebClient).DownloadFile(
			"https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVMVersion/$LLVMArchive",
			"$DepsDir\$LLVMArchive"
		)
	}

	mkdir -Force $LLVMDir
	cmake -E tar xf "$DepsDir\$LLVMArchive"
	Robocopy.exe /move /e "$DepsDir\clang+llvm-$LLVMVersion-x86_64-pc-windows-msvc" $LLVMDir /NFL /NDL /NJH /NJS /NC /NS /NP

	$LLVMDirUnix = $LLVMDir.Replace('\', '/')

	Write-Output "-DLibClang_INCLUDE_DIR=""$LLVMDirUnix/include""" >> $EnvOpts
	Write-Output "-DLibClang_LIBRARY=""$LLVMDirUnix/lib/libclang.lib""" >> $EnvOpts
}

function Add-to-Path {
	$GivenPath = $args[0]

	$NewPath = "$GivenPath;$Env:PATH"
	$Env:PATH = $NewPath

	if ($Null -ne $Env:GITHUB_PATH) {
		$GivenPath >> $Env:GITHUB_PATH
	}
	else {
		[Environment]::SetEnvironmentVariable("PATH", $NewPath, [EnvironmentVariableTarget]::Machine)
	}

	if ($Null -ne $Env:GITHUB_ENV) {
		Write-Output "PATH=$Env:PATH" >> $Env:GITHUB_ENV
	}

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
