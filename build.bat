@echo off
:: Aspace Build Script for Debug and Release configurations
setlocal enabledelayedexpansion

echo ======= Aspace Build System =======

:: Parse input parameter
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=all

:: Set build directories
set DEBUG_DIR=build-debug
set RELEASE_DIR=build-release

if /i "%BUILD_TYPE%"=="debug" goto :debug
if /i "%BUILD_TYPE%"=="release" goto :release
if /i "%BUILD_TYPE%"=="all" goto :all

echo Invalid build type. Use: debug, release, or all
goto :end

:all
echo Building both Debug and Release configurations...
call :debug
call :release
goto :end

:debug
echo Configuring Debug build...
cmake -G Ninja -B %DEBUG_DIR% -DCMAKE_BUILD_TYPE=Debug

if %ERRORLEVEL% NEQ 0 (
    echo Debug configuration failed.
    exit /b %ERRORLEVEL%
)

echo Building Debug build...
cmake --build %DEBUG_DIR%

if %ERRORLEVEL% NEQ 0 (
    echo Debug build failed.
    exit /b %ERRORLEVEL%
)
echo Debug build completed successfully.
goto :eof

:release
echo Configuring Release build...
cmake -G Ninja -B %RELEASE_DIR% -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo Release configuration failed.
    exit /b %ERRORLEVEL%
)

echo Building Release build...
cmake --build %RELEASE_DIR%

if %ERRORLEVEL% NEQ 0 (
    echo Release build failed.
    exit /b %ERRORLEVEL%
)
echo Release build completed successfully.
goto :eof

:end
echo ======= Build Complete =======
endlocal