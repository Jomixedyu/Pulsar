@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%.") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\BuildNinja"
set "CONFIG=Debug"
set "TARGET="
set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

if /I not "%~1"=="" set "CONFIG=%~1"
if /I not "%~2"=="" set "TARGET=%~2"

if not exist "%VSDEVCMD%" (
    echo [Pulsar] VS developer environment script not found:
    echo [Pulsar]   %VSDEVCMD%
    exit /b 1
)

echo [Pulsar] Root: %ROOT_DIR%
echo [Pulsar] Build: %BUILD_DIR%
echo [Pulsar] Config: %CONFIG%
if not "%TARGET%"=="" echo [Pulsar] Target: %TARGET%

if not exist "%BUILD_DIR%\build.ninja" (
    echo [Pulsar] Ninja build files not found, generating first...
    call "%ROOT_DIR%\regen-cmake-ninja.bat"
    if errorlevel 1 goto :fail
)

call "%VSDEVCMD%" -arch=x64 -host_arch=x64
if errorlevel 1 goto :fail

if "%TARGET%"=="" (
    cmake --build "%BUILD_DIR%" --config %CONFIG%
) else (
    cmake --build "%BUILD_DIR%" --config %CONFIG% --target %TARGET%
)
if errorlevel 1 goto :fail

echo.
echo [Pulsar] Ninja build finished successfully.
goto :eof

:fail
echo.
echo [Pulsar] Ninja build failed.
exit /b 1
