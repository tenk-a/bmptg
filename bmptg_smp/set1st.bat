@echo off
if "%1"=="r32" set BmpTgExe=..\bmptg\exe_Release\bmptg.exe
if "%1"=="r64" set BmpTgExe=..\bmptg\exe_Release_X64\bmptg.exe
if "%1"=="d32" set BmpTgExe=..\bmptg\exe_Debug\bmptg.exe
if "%1"=="d64" set BmpTgExe=..\bmptg\exe_Debug_X64\bmptg.exe
if "%1"==""  (
  set BmpTgExe=..\bmptg\bmptg.exe
)
echo set1st.bat [r32,d32,r64 or d64]
echo BmpTgExe=%BmpTgExe%
rem set "PATH=..\bmptg;..\;%PATH%"
