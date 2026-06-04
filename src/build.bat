@echo off
call "D:\vs2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" > nul
cd /d "C:\Users\leoric\Desktop\code\map-inject\src"
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
cmake --build . --config Release
exit /b %ERRORLEVEL%
