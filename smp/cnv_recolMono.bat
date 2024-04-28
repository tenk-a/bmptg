if "%BmpTgExe%"=="" set BmpTgExe=..\bld\bmptg\Release\bmptg.exe
set SRCDIR=src_img
set DSTDIR=dst_recolMono
set CMN_OPTS=:bmp -s%SRCDIR% -if
rem set CMN_OPTS=:png -s%SRCDIR% -b8 -if -cpm2

if not exist %DSTDIR% mkdir %DSTDIR%

call :CONV col002              -b1
call :CONV col004              -b2

call :CONV col002-xde          -b1 -xde
call :CONV col004-xde          -b4 -xde
call :CONV col002-xdp2         -b1 -xdp:2
call :CONV col004-xdp2         -b4 -xdp:2

call :CONV mono016-cp5     -cg -b4 -cp5
call :CONV mono016-xd09x8  -cg -b4 -cp5 -xd9:3

call :CONV mono002-xde     -cg -b1 -xde
call :CONV mono004-xde     -cg -b2 -xde
call :CONV mono008-xde     -cg -b3 -xde
call :CONV mono016-xde     -cg -b4 -xde
call :CONV mono032-xde     -cg -b5 -xde
call :CONV mono064-xde     -cg -b6 -xde

call :CONV mono002-xdp2    -cg -b1 -xdp:2
call :CONV mono004-xdp2    -cg -b2 -xdp:2
call :CONV mono008-xdp2    -cg -b3 -xdp:2
call :CONV mono016-xdp2    -cg -b4 -xdp:2
call :CONV mono032-xdp2    -cg -b5 -xdp:2
call :CONV mono064-xdp2    -cg -b6 -xdp:2

call :CONV mono003jp3-xde  -cg -b3 -cp1 -cn3 -xde
call :CONV mono003jp3-xde2 -cg -b3 -cp1 -cn3 -xde:2

call :CONV mono004jp3-xde  -cg -b3 -cp1 -cn4 -xde
call :CONV mono004jp3-xde2 -cg -b3 -cp1 -cn4 -xde:2
call :CONV mono004jp3-xde3 -cg -b3 -cp1 -cn4 -xde:3

call :CONV mono003jp4-xde  -cg -b4 -cp1 -cn3 -xde
call :CONV mono004jp4-xde0 -cg -b4 -cp1 -cn4 -xde:0

call :CONV c1mono003jp3-xde -cg:0000ff -b3 -cp1 -cn3 -xde
call :CONV c2mono003jp3-xde -cg:ff0000 -b3 -cp1 -cn3 -xde
call :CONV c3mono003jp3-xde -cg:ff00ff -b3 -cp1 -cn3 -xde
call :CONV c4mono003jp3-xde -cg:00ffff -b3 -cp1 -cn3 -xde
call :CONV c5mono003jp3-xde -cg:ffff00 -b3 -cp1 -cn3 -xde
call :CONV c6mono003jp3-xde -cg:ffffff -b3 -cp1 -cn3 -xde

call :CONV c1mono003jp4-xde -cg:0000ff -b4 -cp1 -cn3 -xde
call :CONV c2mono003jp4-xde -cg:ff0000 -b4 -cp1 -cn3 -xde
call :CONV c3mono003jp4-xde -cg:ff00ff -b4 -cp1 -cn3 -xde
call :CONV c4mono003jp4-xde -cg:00ffff -b4 -cp1 -cn3 -xde
call :CONV c5mono003jp4-xde -cg:ffff00 -b4 -cp1 -cn3 -xde
call :CONV c6mono003jp4-xde -cg:ffffff -b4 -cp1 -cn3 -xde

call :CONV c1mono004jp4-xde -cg:0000ff -b4 -cp1 -cn4 -xde
call :CONV c2mono004jp4-xde -cg:ff0000 -b4 -cp1 -cn4 -xde
call :CONV c3mono004jp4-xde -cg:ff00ff -b4 -cp1 -cn4 -xde
call :CONV c4mono004jp4-xde -cg:00ffff -b4 -cp1 -cn4 -xde
call :CONV c5mono004jp4-xde -cg:ffff00 -b4 -cp1 -cn4 -xde
call :CONV c6mono004jp4-xde -cg:ffffff -b4 -cp1 -cn4 -xde


goto END

:CONV
set TGTDIR=%DSTDIR%\%1
set OPTS=%2 %3 %4 %5 %6 %7 %8 %9
if not exist %TGTDIR% mkdir %TGTDIR%
%BmpTgExe% %CMN_OPTS% -d%TGTDIR% *.png *.jpg %OPTS%
exit /b

:END
