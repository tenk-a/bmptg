if "%BmpTgExe%"=="" set BmpTgExe=..\bmptg\exe_Release_x64\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recol
set CMN_OPTS=:png -s%SRCDIR% -b8 -if

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\col2
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg -if -b1

set TGTDIR=%DSTDIR%\col4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg -if -b2

set TGTDIR=%DSTDIR%\col16mcyuv
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% :bmp -d%TGTDIR% *.png *.jpg -if -b4

set TGTDIR=%DSTDIR%\fix256jp
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -cp1

set TGTDIR=%DSTDIR%\fix256jp-xd03-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd3:3 -cp1

set TGTDIR=%DSTDIR%\fix256jp-xd06-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd6:3 -cp1

set TGTDIR=%DSTDIR%\fix256jp-xd09-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:3 -cp1

set TGTDIR=%DSTDIR%\fix256jp-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd12:3 -cp1

set TGTDIR=%DSTDIR%\fix256win
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -cp2

set TGTDIR=%DSTDIR%\fix256win-xd09-x4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg  -xd9:2 -cp2

set TGTDIR=%DSTDIR%\fix256win-xd09-x8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg  -xd9:3 -cp2

set TGTDIR=%DSTDIR%\fix256win-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg  -xd12:3 -cp2


rem set TGTDIR=%DSTDIR%\fix256rgb_7x6x6m4
rem if not exist %TGTDIR% mkdir %TGTDIR%
rem %BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -cp7

rem set TGTDIR=%DSTDIR%\fix256rgb_7x6x6m4-xd09-2x2
rem if not exist %TGTDIR% mkdir %TGTDIR%
rem %BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:1 -cp7

rem set TGTDIR=%DSTDIR%\fix256rgb_7x6x6m4-xd09-4x4
rem if not exist %TGTDIR% mkdir %TGTDIR%
rem %BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:2 -cp7

rem set TGTDIR=%DSTDIR%\fix256rgb_7x6x6m4-xd09-8x8
rem if not exist %TGTDIR% mkdir %TGTDIR%
rem %BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:3 -cp7

rem set TGTDIR=%DSTDIR%\fix256rgb_7x6x6m4-xd12-8x8
rem if not exist %TGTDIR% mkdir %TGTDIR%
rem %BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:3 -cp7

set TGTDIR=%DSTDIR%\fix256rgb_6x6x6c40
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -cp8

set TGTDIR=%DSTDIR%\fix256rgb_6x6x6c40-xd09-2x2
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:1 -cp8

set TGTDIR=%DSTDIR%\fix256rgb_6x6x6c40-xd09-4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:2 -cp8

set TGTDIR=%DSTDIR%\fix256rgb_6x6x6c40-xd09-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:3 -cp8

set TGTDIR=%DSTDIR%\fix256rgb_6x6x6c40-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -xd9:3 -cp8


set TGTDIR=%DSTDIR%\mc-yuv
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -cp3

set TGTDIR=%DSTDIR%\mc-yuv-xd12-2x2
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap  -xd12:1 -cp3

set TGTDIR=%DSTDIR%\mc-yuv-xd12-4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap  -xd12:2 -cp3

set TGTDIR=%DSTDIR%\mc-yuv-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap  -xd12:3 -cp3

set TGTDIR=%DSTDIR%\mc-yuv-xd15-4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap  -xd15:2 -cp3

set TGTDIR=%DSTDIR%\mc-rgb
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -cp4

set TGTDIR=%DSTDIR%\mc-rgb-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap  -xd12:3 -cp4

set TGTDIR=%DSTDIR%\hist
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -cp5

set TGTDIR=%DSTDIR%\hist-xd12-2x2
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd12:1 -cp5

set TGTDIR=%DSTDIR%\hist-xd12-4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd12:2 -cp5

set TGTDIR=%DSTDIR%\hist-xd12-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd12:3 -cp5

set TGTDIR=%DSTDIR%\hist-xd15-2x2
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd15:1 -cp5

set TGTDIR=%DSTDIR%\hist-xd15-4x4
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd15:2 -cp5

set TGTDIR=%DSTDIR%\hist-xd15-8x8
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg -ap -xd15:3 -cp5
