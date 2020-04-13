rem %1 ‚Í -xrc or -xrb ‚ð‘z’è
if "%BmpTgExe%"=="" set BmpTgExe=..\bmptg\exe_Release_x64\bmptg.exe
set SRCDIR=src_img_for_resize_to_large
set DSTDIR=dst_resize_to_large
set CMN_OPTS=:png -s%SRCDIR% -if

if not exist %DSTDIR% mkdir %DSTDIR%

set XRP=xrp600
set TGTDIR=%DSTDIR%\%XRP%
if not exist %TGTDIR% mkdir %TGTDIR%

rem %BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xre
rem abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_e2.png
rem goto :EOF

%BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xrn
abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_a.png
%BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xrb
abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_b.png
%BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xrc
abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_c.png
%BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xre
abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_e.png
%BmpTgExe% %CMN_OPTS% *.jpg *.png -%XRP% -d%TGTDIR%\tmp -xrl
abx -s -y -x %TGTDIR%\tmp\*.png =move $f %TGTDIR%\$x_l.png
