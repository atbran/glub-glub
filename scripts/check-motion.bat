@echo off
setlocal
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b %errorlevel%
"%ProgramFiles%\CMake\bin\cmake.exe" -S . -B build-msvc -DGLUB_BUILD_MOTION_CHECKS=ON
if errorlevel 1 exit /b %errorlevel%
"%ProgramFiles%\CMake\bin\cmake.exe" --build build-msvc --target KoiMotionChecks --parallel 4
if errorlevel 1 exit /b %errorlevel%
build-msvc\KoiMotionChecks_artefacts\Release\KoiMotionChecks.exe
exit /b %errorlevel%
