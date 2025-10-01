@echo off

if "%1"=="r" set BmpTgExe=..\bin\exe64\bmptg.exe
if "%1"=="d" set BmpTgExe=..\bld\bmptg-vc143x64\Debug\bmptg.exe
if "%1"==""  (
  set BmpTgExe=..\bin\exe64\bmptg.exe
)
echo set1st.bat [r or d]
echo BmpTgExe=%BmpTgExe%
rem set "PATH=..\bmptg;..\;%PATH%"
