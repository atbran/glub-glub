@echo off
REM Glub-Glub Windows build (run from repo root in cmd.exe, NOT PowerShell quoting hell)
REM Requires: Visual Studio 2022 Build Tools with "Desktop C++" + CMake
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
echo.
echo VST3: build\GlubGlub_artefacts\Release\VST3\Glub-Glub.vst3
echo EXE:  build\GlubGlub_artefacts\Release\Standalone\Glub-Glub.exe
