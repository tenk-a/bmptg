rem %1 ‚Í -xrc or -xrb ‚ð‘z’è
if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
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
