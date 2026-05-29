$RECOMMENDED_CLANG_FORMAT_MAJOR="11"

function Get-ClangFormatVersion {
    $program = $(Get-Command clang-format.exe -ErrorAction SilentlyContinue)
    if ($program) {
        $version = & $program.Source --version
        $clangFormatVersion = $version.Split(" ")[2]
        $clangMajorVersion = $clangFormatVersion.Split(".")[0]
        if ($clangMajorVersion -ge $RECOMMENDED_CLANG_FORMAT_MAJOR) {
            return $clangMajorVersion
        }
    }
}

$CLANG_FORMAT = Get-ClangFormatVersion