@echo off
cd libjpeg-turbo
if exist Makefile (
  nmake clean
  del Makefile
)
if exist CMakeFiles (
  del /S /F /Q CMakeFiles\*.*
  rmdir /S /Q CMakeFiles
)
if exist CMakeCache.txt      del CMakeCache.txt
if exist cmake_install.cmake del cmake_install.cmake
if exist CTestTestfile.cmake del CTestTestfile.cmake
if exist libjpeg-turbo.nsi   del libjpeg-turbo.nsi
if exist turbojpeg.exp	     del turbojpeg.exp

cd simd
if exist Makefile (
  nmake clean
  del Makefile
)
if exist CMakeFiles (
  del /S /F /Q CMakeFiles\*.*
  rmdir /S /Q CMakeFiles
)
if exist cmake_install.cmake del cmake_install.cmake
cd ..
cd sharedlib
if exist Makefile (
  nmake clean
  del Makefile
)
if exist CMakeFiles (
  del /S /F /Q CMakeFiles\*.*
  rmdir /S /Q CMakeFiles
)
if exist cmake_install.cmake del cmake_install.cmake
if exist jpeg.exp	     del jpeg.exp
cd ..

cd ..
