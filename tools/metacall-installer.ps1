# metacall-installer.ps1 — MetaCall Windows Installer
# Usage: .\metacall-installer.ps1

Write-Host "MetaCall Windows Installer" -ForegroundColor Cyan

# Install LLVM
Write-Host "Installing LLVM..." -ForegroundColor Yellow
choco install llvm --yes --no-progress

# Install libffi via vcpkg
Write-Host "Installing libffi..." -ForegroundColor Yellow
if (-not (Test-Path "C:\vcpkg")) {
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    cmd /c "C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics"
}
C:\vcpkg\vcpkg.exe install libffi:x64-windows

# Install TCC
Write-Host "Installing TCC..." -ForegroundColor Yellow
Invoke-WebRequest -Uri "https://github.com/metacall/tinycc/archive/4fccaf61241a5eb72b0777b3a44bd7abbea48604.zip" `
    -OutFile "$env:TEMP\tinycc.zip" -UseBasicParsing
Expand-Archive -Path "$env:TEMP\tinycc.zip" -DestinationPath "C:\tinycc_src" -Force
$inner = Get-ChildItem "C:\tinycc_src" -Directory | Select-Object -First 1
cd "$($inner.FullName)\win32"
.\build-tcc.bat -i C:\mc_out

# Set environment variables
[Environment]::SetEnvironmentVariable("LLVM_ROOT", "C:\Program Files\LLVM", "Machine")
[Environment]::SetEnvironmentVariable("TCC_ROOT", "C:\mc_out", "Machine")
[Environment]::SetEnvironmentVariable("LIBFFI_ROOT", "C:\vcpkg\installed\x64-windows", "Machine")

Write-Host "Done! MetaCall C Loader dependencies installed." -ForegroundColor Green
Write-Host "LLVM_ROOT = C:\Program Files\LLVM"
Write-Host "TCC_ROOT  = C:\mc_out"
