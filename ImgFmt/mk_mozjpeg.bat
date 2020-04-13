@echo off
setlocal

set BLD_TYPE=
set BLD_SRC_NAME=
set BLD_DST_NAME=
if "%1"=="32" (
 set BLD_TYPE=Release
  set BLD_SRC_NAME=turbojpeg-static.lib
  set BLD_DST_NAME=turbojpeg-static32.lib
)
if "%1"=="64" (
  set BLD_TYPE=Release
  set BLD_SRC_NAME=turbojpeg-static.lib
  set BLD_DST_NAME=turbojpeg-static64.lib
)
if "%1"=="32d" (
  set BLD_TYPE=Debug
  set BLD_SRC_NAME=turbojpeg-static.lib
  set BLD_DST_NAME=turbojpeg-static32d.lib
)
if "%1"=="64d" (
  set BLD_TYPE=Debug
  set BLD_SRC_NAME=turbojpeg-static.lib
  set BLD_DST_NAME=turbojpeg-static64d.lib
)
if "%BLD_TYPE%"=="" (
  echo mk_turbojpeg [32,64,32d,64d]
  echo 32,64 : relase  32d,64d : debug
  goto END
)


cd mozjpeg

set bld=bid%1
if not exist %bld% mkdir %bld%
cd %bld%
del /f /q /s *.*

CMake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BLD_TYPE% ..\

call :ReplaceMDtoMT

nmake turbojpeg-static
copy %BLD_SRC_NAME% ..\%BLD_DST_NAME%
if not exist ..\jconfig.h copy jconfig.h ..\

cd ..
cd ..

goto END


:ReplaceMTtoMD
for /R %%i in (CMakeCache.txt) do (
  if exist %%i call :Rep1MTtoMD %%i
)
exit /b
:Rep1MTtoMD
set TgtReplFile=%1
set BakReplFile=%1.bak
if exist %BakReplFile% del %BakReplFile%
move %TgtReplFile% %BakReplFile%
type nul >%TgtReplFile%
for /f "delims=" %%A in (%BakReplFile%) do (
    set line=%%A
    call :Rep1SubMTtoMD
)
exit /b
:Rep1SubMTtoMD
echo %line:/MT=/MD%>>%TgtReplFile%
exit /b


:ReplaceMDtoMT
for /R %%i in (CMakeCache.txt) do (
  if exist %%i call :Rep1MDtoMT %%i
)
exit /b
:Rep1MDtoMT
set TgtReplFile=%1
set BakReplFile=%1.bak
if exist %BakReplFile% del %BakReplFile%
move %TgtReplFile% %BakReplFile%
type nul >%TgtReplFile%
for /f "delims=" %%A in (%BakReplFile%) do (
    set line=%%A
    call :Rep1SubMDtoMT
)
exit /b
:Rep1SubMDtoMT
echo %line:/MD=/MT%>>%TgtReplFile%
exit /b


:END
endlocal
