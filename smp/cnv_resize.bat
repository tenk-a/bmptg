rem %1 ‚Í -xrc or -xrb ‚ð‘z’è
if "%BmpTgExe%"=="" set BmpTgExe=..\bld\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_resize%1
set CMN_OPTS=:png -s%SRCDIR% %1 -if

if not exist %DSTDIR% mkdir %DSTDIR%

set TGTDIR=%DSTDIR%\xrp100
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp100

set TGTDIR=%DSTDIR%\xrp010
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp010

set TGTDIR=%DSTDIR%\xrp033
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp33

set TGTDIR=%DSTDIR%\xrp050
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp50

set TGTDIR=%DSTDIR%\xrp080
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp80

set TGTDIR=%DSTDIR%\xrp050x100
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp50:100

set TGTDIR=%DSTDIR%\xrp050x150
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp50:150

set TGTDIR=%DSTDIR%\xrp100x050
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp100:50

set TGTDIR=%DSTDIR%\xrp100x150
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp100:150

set TGTDIR=%DSTDIR%\xrp100x200
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp100:200

set TGTDIR=%DSTDIR%\xrp150x050
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp150:50

set TGTDIR=%DSTDIR%\xrp150x100
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp150:100

set TGTDIR=%DSTDIR%\xrp150
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp150

set TGTDIR=%DSTDIR%\xrp200
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp200

set TGTDIR=%DSTDIR%\xrp200x050
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp200:50

set TGTDIR=%DSTDIR%\xrp200x100
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp200:100

set TGTDIR=%DSTDIR%\xrp300
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.jpg *.png -xrp300
