@echo off
setlocal
REM MSVC + Ninja build. This avoids the Visual Studio generator's VCTargetsPath probe.
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%

"%ProgramFiles%\CMake\bin\cmake.exe" --fresh -S . -B build-msvc -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe
if errorlevel 1 exit /b %errorlevel%

"%ProgramFiles%\CMake\bin\cmake.exe" --build build-msvc --parallel
if errorlevel 1 exit /b %errorlevel%

echo.
echo VST3: build-msvc\GlubGlub_artefacts\Release\VST3\Glub-Glub.vst3
echo EXE:  build-msvc\GlubGlub_artefacts\Release\Standalone\Glub-Glub.exe
exit /b 0
