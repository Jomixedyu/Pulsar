@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%.") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\BuildNinja"
set "CONFIG=Debug"
set "TARGET=PulsarEd"
set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

echo [Pulsar] Root: %ROOT_DIR%
echo [Pulsar] Build: %BUILD_DIR%
echo [Pulsar] Config: %CONFIG%
echo [Pulsar] Target: %TARGET%

if not exist "%BUILD_DIR%\build.ninja" (
    echo [Pulsar] Ninja build files not found, generating first...
    call "%ROOT_DIR%\regen-cmake-ninja.bat"
    if errorlevel 1 goto :fail
)

call "%VSDEVCMD%" -arch=x64 -host_arch=x64
if errorlevel 1 goto :fail

cmake --build "%BUILD_DIR%" --config %CONFIG% --target %TARGET% --parallel
if errorlevel 1 goto :fail

echo.
echo [Pulsar] Build finished successfully.
goto :eof

:fail
echo.
echo [Pulsar] Build failed.
exit /b 1
