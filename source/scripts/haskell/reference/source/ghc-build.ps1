param(
	[string]$OutputPath = ""
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $scriptDir ".ghc-build"

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
	$OutputPath = Join-Path $scriptDir "metacall_hs_bridge.dll"
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$exportSymbols = @(
	"metacall_hs_module_initialize",
	"metacall_hs_module_destroy",
	"metacall_hs_module_function_count",
	"metacall_hs_module_function_name",
	"metacall_hs_module_function_args_count",
	"metacall_hs_module_function_arg_name",
	"metacall_hs_module_function_arg_type",
	"metacall_hs_module_function_return_type",
	"metacall_hs_module_invoke",
	"metacall_hs_module_value_destroy",
	"metacall_hs_module_last_error"
)

$ghcArgs = @(
	"--make",
	"-shared",
	"-no-hs-main",
	"-outputdir", $buildDir,
	(Join-Path $scriptDir "MetacallHSBridge.hs"),
	"-optl-Wl,--export-all-symbols"
)

foreach ($symbol in $exportSymbols) {
	$ghcArgs += "-optl-Wl,--undefined=$symbol"
}

$ghcArgs += @("-o", $OutputPath)

& ghc @ghcArgs

if ($LASTEXITCODE -ne 0) {
	exit $LASTEXITCODE
}

Write-Host "Built: $OutputPath"
