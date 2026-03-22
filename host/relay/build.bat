@echo off
REM =============================================================================
REM TesseraRelay.dll build script — requires Visual C++ Toolkit 2003
REM
REM Prerequisites:
REM   1. Install Visual C++ Toolkit 2003
REM   2. Generate CRT import libraries (one-time):
REM      Run generate_crt_libs.bat to create msvcrt.lib and msvcprt.lib
REM      from the msvcr71.dll/msvcp71.dll in the game directory.
REM =============================================================================

set VCDIR=C:\Program Files (x86)\Microsoft Visual C++ Toolkit 2003

if not exist "%VCDIR%\bin\cl.exe" (
    echo ERROR: Visual C++ Toolkit 2003 not found at %VCDIR%
    exit /b 1
)

if not exist "%~dp0lib71\msvcrt.lib" (
    echo ERROR: CRT import libraries not found in lib71\
    echo Run generate_crt_libs.bat first.
    exit /b 1
)

set PATH=%VCDIR%\bin;%PATH%
set INCLUDE=%VCDIR%\include
set LIB=%VCDIR%\lib;%~dp0lib71

echo Building TesseraRelay.dll with VS2003 (MSVC 7.1)...

cd /d "%~dp0"

REM Step 1: Compile .obj files
cl /nologo /c /O2 /EHsc /MD /DRELAY_EXPORTS ^
    /I include /I src ^
    src\relay_main.cpp ^
    src\relay_xml.cpp ^
    src\relay_utility.cpp ^
    src\relay_stubs.cpp

if %ERRORLEVEL% neq 0 (
    echo COMPILE FAILED
    exit /b 1
)

REM Step 2: Link into build\Debug (primary output)
link /nologo /DLL ^
    /OUT:..\build\Debug\TesseraRelay.dll ^
    relay_main.obj relay_xml.obj relay_utility.obj relay_stubs.obj ^
    msvcrt.lib msvcprt.lib libcmt.lib libcpmt.lib kernel32.lib ^
    /FORCE:MULTIPLE

if %ERRORLEVEL% neq 0 (
    echo LINK FAILED
    exit /b 1
)

REM Copy to Release dir too (same binary — VS2003 has no debug/release split)
if not exist "..\build\Release" mkdir "..\build\Release"
copy /y "..\build\Debug\TesseraRelay.dll" "..\build\Release\TesseraRelay.dll" >nul

echo.
echo SUCCESS: build\Debug\TesseraRelay.dll (+ copied to build\Release\)
echo.

REM Cleanup intermediate files
del /q *.obj *.exp 2>nul

REM Verify exports
echo Exports:
dumpbin /exports ..\build\Debug\TesseraRelay.dll | findstr "relay_"

echo.
echo Dependencies (should show MSVCR71.dll and MSVCP71.dll):
dumpbin /dependents ..\build\Debug\TesseraRelay.dll | findstr -i "msvc"
