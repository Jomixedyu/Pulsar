@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%.") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\BuildNinja"
set "GENERATOR=Ninja"
set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

if not exist "%VSDEVCMD%" (
    echo [Pulsar] VS developer environment script not found:
    echo [Pulsar]   %VSDEVCMD%
    exit /b 1
)

echo [Pulsar] Root: %ROOT_DIR%
echo [Pulsar] Build: %BUILD_DIR%
echo [Pulsar] Generator: %GENERATOR% + MSVC

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if errorlevel 1 goto :fail

call "%VSDEVCMD%" -arch=x64 -host_arch=x64
if errorlevel 1 goto :fail

cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe
if errorlevel 1 goto :fail

echo.
echo [Pulsar] Ninja CMake generate finished successfully.
goto :eof

:fail
echo.
echo [Pulsar] Ninja CMake generate failed.
exit /b 1
