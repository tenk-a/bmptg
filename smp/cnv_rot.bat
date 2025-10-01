rem %1 は -xrc or -xrb を想定...
if "%BmpTgExe%"=="" set BmpTgExe=..\bin\exe64\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_rot%1
set CMN_OPTS=:png -s%SRCDIR% %1 -if

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\rot45
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xi45

set TGTDIR=%DSTDIR%\rotM45
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xi-45


set TGTDIR=%DSTDIR%\rot30
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xi30

set TGTDIR=%DSTDIR%\rotM60
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xi-60
