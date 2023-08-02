# Extract the OS name from the first argument
$env:OS_NAME = $Global:Arguments[0] -replace '-2019$', ''
# Extract the benchmark name from the second argument
$env:BENCH_NAME = $Global:Arguments[1] -replace 'metacall-(.*?)-bench.json', '$1'
# Set the OS_NAME environment variable for use in subsequent steps
$envVariables = "OS_NAME=$env:OS_NAME`nBENCH_NAME=$env:BENCH_NAME`nBENCH_PATH=./build/Debug/"
$envVariables | Out-File -FilePath $env:GITHUB_ENV -Append
