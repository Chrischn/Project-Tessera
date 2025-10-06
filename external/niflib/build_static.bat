@echo off
setlocal enabledelayedexpansion
REM ===== NIFLIB static (no DLL) for Debug & Release build =====

REM
cd /d "%~dp0"

REM ----- VS-Environment automatically set -----
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSINST="

if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%I in (`
    "%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  `) do set "VSINST=%%I"
)

if defined VSINST (
  if exist "%VSINST%\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VSINST%\VC\Auxiliary\Build\vcvarsall.bat" x64
  ) else if exist "%VSINST%\Common7\Tools\VsDevCmd.bat" (
    call "%VSINST%\Common7\Tools\VsDevCmd.bat" -arch=amd64
  ) else (
    echo [ERROR] VS-Toolchain not found. >&2
    exit /b 1
  )
) else (
  REM Fallback: try a few standard paths
  for %%P in (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
  ) do (
    if exist %%P (
      call %%P -arch=amd64
      set "VSINST=FALLBACK"
      goto :vs_ok
    )
  )
  echo [ERROR] Visual Studio 2022 Build Tools not found. Please install VS2022 with C++-Workload. >&2
  exit /b 1
)
:vs_ok

REM ----- CMake-Configuration (Multi-Config, static) -----
set "BUILD_DIR=%cd%\build_vs2022_static"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM /MD (Release) and /MDd (Debug):
set "RUNTIME=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"

echo === Configure (static + /MD/MDd) ===
cmake -S "%cd%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DCMAKE_C_FLAGS_DEBUG=/MDd -DCMAKE_CXX_FLAGS_DEBUG=/MDd ^
  -DCMAKE_C_FLAGS_RELEASE=/MD -DCMAKE_CXX_FLAGS_RELEASE=/MD
if errorlevel 1 goto :err

echo === Build Debug ===
cmake --build "%BUILD_DIR%" --config Debug
if errorlevel 1 goto :err

echo === Build Release ===
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 goto :err

echo.
echo === Finished. Created static .lib: ===
echo Debug:
dir /b "%BUILD_DIR%\Debug\*.lib"
echo.
echo Release:
dir /b "%BUILD_DIR%\Release\*.lib"

exit /b 0

:err
echo *** Build failed (Error %errorlevel%) ***
exit /b %errorlevel%
