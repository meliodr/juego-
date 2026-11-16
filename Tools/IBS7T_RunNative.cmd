@echo off
setlocal
rem Native-only, one translation unit. No UHT, UBT, cook or engine process.
if not defined IBS7T_VCVARS set "IBS7T_VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%IBS7T_VCVARS%" exit /b 2
call "%IBS7T_VCVARS%" >nul
if errorlevel 1 exit /b 2
set "IBS7T_OUT=%TEMP%\IBS7T_Native_%RANDOM%_%RANDOM%"
mkdir "%IBS7T_OUT%"
if errorlevel 1 exit /b 2
cl /nologo /std:c++17 /EHsc /W4 /WX /Od /I"%~dp0..\ImpactBeyond\Source\ImpactBeyond" "%~dp0IBS7T_NativeTests.cpp" /Fo"%IBS7T_OUT%\tests.obj" /Fe"%IBS7T_OUT%\tests.exe"
if errorlevel 1 exit /b 1
"%IBS7T_OUT%\tests.exe"
exit /b %ERRORLEVEL%
