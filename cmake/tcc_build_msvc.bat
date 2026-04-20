@echo off
set "SRCDIR=%~1"
set "OUTDIR=%~2"

cd /d "%SRCDIR%\win32"

call build-tcc.bat -i "%OUTDIR%"

:: Ignore xcopy errors, copy manually
if not exist "%OUTDIR%" mkdir "%OUTDIR%"
xcopy /s /i /q /y "include" "%OUTDIR%\include\" 2>nul
xcopy /s /i /q /y "lib" "%OUTDIR%\lib\" 2>nul
copy /y "*.dll" "%OUTDIR%\" 2>nul
copy /y "*.exe" "%OUTDIR%\" 2>nul

exit /B 0
