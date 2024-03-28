pushd %~dp0

set BLD_ARCH=%1
set BLD_TYPE=%2

if "%BLD_ARCH%"=="" set BLD_ARCH=x64
if "%BLD_TYPE%"=="" set BLD_TYPE=Release

set ThirdParty=..\..\..

if not exist lib mkdir lib
pushd lib

set LibDir=%BLD_ARCH%_%BLD_TYPE%

if not exist %LibDir% mkdir %LibDir%
pushd %LibDir%

:: goto SKIP_ZLIB
rem
rem zlib
rem
set ZlibDir=%ThirdParty%\zlib
set ZlibWork=zlib
if exist %ZlibWork% rmdir /s /q %ZlibWork%
if not exist %ZlibWork% mkdir %ZlibWork%
pushd %ZlibWork%
 copy %ZlibDir%\zconf.h %ZlibDir%\zconf.h.tmp
 CMake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BLD_TYPE% %ZlibDir%
 nmake
 move %ZlibDir%\zconf.h.tmp %ZlibDir%\zconf.h
popd
:SKIP_ZLIB

:: goto SKIP_LIBPNG
rem
rem libpng
rem
set LibpngDir=%ThirdParty%\libpng
set PngWork=libpng
if exist %PngWork% rmdir /s /q %PngWork%
if not exist %PngWork% mkdir %PngWork%
pushd %PngWork%
 CMake -G "NMake Makefiles" ^
 	-DCMAKE_BUILD_TYPE=%BLD_TYPE%	^
 	-DPNG_STATIC=on	^
 	-DZLIB_LIBRARY=..\%ZlibWork%\zlibstatic.lib	^
 	-DZLIB_INCLUDE_DIR=%ZlibDir%	^
	%LibpngDir%
 nmake
 copy %LibpngDir%\scripts\pnglibconf.h.prebuilt %LibpngDir%\pnglibconf.h
popd
:SKIP_LIBPNG

:: goto SKIP_LIBJPEG
rem
rem libjpeg-turbo
rem
set JpgDir=%ThirdParty%\libjpeg-turbo
set JpgWork=libjpeg-turbo
if exist %JpgWork% rmdir /s /q %JpgWork%
if not exist %JpgWork% mkdir %JpgWork%
pushd %JpgWork%
 CMake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BLD_TYPE% %JpgDir%
 nmake turbojpeg-static
popd
:SKIP_LIBJPEG

popd
popd
popd
