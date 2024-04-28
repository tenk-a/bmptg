if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_book
set DSTDIR=dst_book
set JPG_OPTS=:jpg -xq80 -xqg33
set CMN_OPTS=:png -cfn112288

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\autogrey
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %JPG_OPTS% -d%TGTDIR% -s%SRCDIR% *.png -cgn

set TGTDIR=%DSTDIR%\left
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xv+100 *.png

set TGTDIR=%DSTDIR%\right
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvr+100 *.png

set TGTDIR=%DSTDIR%\center
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvc+100 *.png

set TGTDIR=%DSTDIR%\center_up
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvcu+100:+100 *.png

set TGTDIR=%DSTDIR%\center_center
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvcm+100:+100 *.png

set TGTDIR=%DSTDIR%\center_down
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvcd+100:+100 *.png

set TGTDIR=%DSTDIR%\lcr_ex
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvem+100:+100 *.png

set TGTDIR=%DSTDIR%\xv2nd
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% -s%SRCDIR% -xvc-64:-64 -xvem+100:+100 *.png
