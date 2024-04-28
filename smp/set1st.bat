@echo off

if "%1"=="r" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
if "%1"=="d" set BmpTgExe=..\bld\bmptg\Debug\bmptg.exe
if "%1"==""  (
  set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
)
echo set1st.bat [r or d]
echo BmpTgExe=%BmpTgExe%
rem set "PATH=..\bmptg;..\;%PATH%"
