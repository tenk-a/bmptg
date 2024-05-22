if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_paint
set DSTDIR=dst_paint
set CMN_OPTS=:png -s%SRCDIR% -if

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\xca
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xca
