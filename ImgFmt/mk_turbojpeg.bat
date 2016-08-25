@echo off
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

call clean_turbojpeg.bat

cd turbojpeg
CMake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BLD_TYPE%
nmake turbojpeg-static
copy %BLD_SRC_NAME% %BLD_DST_NAME%
cd ..

:END
set BLD_TYPE=
set BLD_SRC_NAME=
set BLD_DST_NAME=
