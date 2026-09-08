@echo off
setlocal
REM Demo-variant build: synthesizes a 128 BPM groove to drive the visuals.
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%
"%ProgramFiles%\CMake\bin\cmake.exe" --fresh -S . -B build-demo -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe -DGLUB_DEMO_MODE=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=%~dp0..\build-msvc\_deps\juce-src
if errorlevel 1 exit /b %errorlevel%
"%ProgramFiles%\CMake\bin\cmake.exe" --build build-demo --parallel
if errorlevel 1 exit /b %errorlevel%
echo EXE: build-demo\GlubGlub_artefacts\Release\Standalone\Glub-Glub.exe
exit /b 0
