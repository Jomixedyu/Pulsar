@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%.") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\BuildNinja"
set "EDITOR_EXE=%BUILD_DIR%\Source\PulsarEd\PulsarEd.exe"

call "%ROOT_DIR%\build-debug-pulsared.bat"
if errorlevel 1 exit /b 1

if not exist "%EDITOR_EXE%" (
    echo [Pulsar] Editor executable not found:
    echo [Pulsar]   %EDITOR_EXE%
    exit /b 1
)

echo [Pulsar] Working directory: %BUILD_DIR%
echo [Pulsar] Launching: %EDITOR_EXE% %*
pushd "%BUILD_DIR%"
"%EDITOR_EXE%" %*
set "EXIT_CODE=%ERRORLEVEL%"
popd

exit /b %EXIT_CODE%
