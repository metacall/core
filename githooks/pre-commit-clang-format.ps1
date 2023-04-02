$RECOMMENDED_CLANG_FORMAT_MAJOR="11"

function Get-ClangFormatVersion {
    $program = $(Get-Command clang-format -ErrorAction SilentlyContinue)
    $clangFormatVersion = $program.Version
    $program = $program.Source
    $clangMajorVersion = $clangFormatVersion.Split(" ")[2].Split(".")[0]
    if ($clangMajorVersion -ge $RECOMMENDED_CLANG_FORMAT_MAJOR) {
        return $program
    }
}

$CLANG_FORMAT = Get-ClangFormatVersion