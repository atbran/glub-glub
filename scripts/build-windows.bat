@echo off
REM Compatibility entry point. The MSVC + Ninja path is deterministic on Windows.
call "%~dp0build-msvc-ninja.bat"
exit /b %errorlevel%
