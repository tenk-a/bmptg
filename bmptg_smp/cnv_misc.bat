if "%BmpTgExe%"=="" set BmpTgExe=..\bmptg\exe_Release_x64\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_misc
set CMN_OPTS=:png -s%SRCDIR% -if

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\mono
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -cg

set TGTDIR=%DSTDIR%\tone050
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -t50

set TGTDIR=%DSTDIR%\tone066
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -t66

set TGTDIR=%DSTDIR%\tone150
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -t200

set TGTDIR=%DSTDIR%\tone050i
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -tt50

set TGTDIR=%DSTDIR%\tone066i
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -tt66

set TGTDIR=%DSTDIR%\tone150i
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -tt200

set TGTDIR=%DSTDIR%\mulcolF0C890
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -cmF0C890

set TGTDIR=%DSTDIR%\rgbScale10-9-6
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -cs1.0:1.0:0.9:0.6

set TGTDIR=%DSTDIR%\yuvScale10-03-13
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -ct1.0:1.0:0.3:1.3

set TGTDIR=%DSTDIR%\filter1n3
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xf1:3

set TGTDIR=%DSTDIR%\revx
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xx

set TGTDIR=%DSTDIR%\revy
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xy

set TGTDIR=%DSTDIR%\revxy
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xx -xy

set TGTDIR=%DSTDIR%\rot90
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xj

